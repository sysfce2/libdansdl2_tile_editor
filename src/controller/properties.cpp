#include "controller/properties.h"
#include "input/input.h"
#include "app/definitions.h"
#include "app/entity_inflator.h"

#include <ldv/ttf_representation.h>
#include <ldv/box_representation.h>

#include <algorithm>

using namespace controller;

properties::properties(
	lm::logger& _log,
	ldtools::ttf_manager& _ttf_manager,
	tile_editor::exchange_data& _exchange_data
)
	:log{_log},
	ttf_manager{_ttf_manager},
	exchange_data{_exchange_data} {

}

void properties::awake(
	dfw::input& /*input*/,
	int
) {

	if(!exchange_data.has(state_properties)) {

		throw std::runtime_error("could not find data for state_properties");
	}

	exchange_data.recover(state_properties);
	property_manager=exchange_data.properties;
	property_manager_backup=*exchange_data.properties;
	blueprint=exchange_data.properties_blueprint;

	//Each entry will be assigned a numeric key. A separate vector will keep
	//track of the relationship between each key and the name-type of the
	//property for lookup purposes.
	auto add=[this](const auto& _props, option_types _type) {

		for(const auto& pair : _props) {

			lookup.push_back({_type, pair.first});
			lm::log(log).info()<<"lookup added: "<<pair.first<<std::endl;
		}
	};

	lookup.clear();

	//TODO: Please, make them appear in the same order instead of this!!!
	add(property_manager->string_properties, option_types::str);
	add(property_manager->int_properties, option_types::integer);
	add(property_manager->double_properties, option_types::decimal);

	lm::log(log).info()<<"lookup size: "<<lookup.size()<<std::endl;

	//TODO: And what if the map contains more info than the properties exposed
	//in the config file?.

	//Now we could sort them by key, I guess... TODO: But that is bull,
	//what we would really want is for them to appear in the SAME ORDER.
	const auto& names=blueprint->property_names;
	
	std::sort(
		std::begin(lookup),
		std::end(lookup),
		[&names](const auto& _a, const auto& _b) {

			auto a_it=std::distance(
					std::begin(names),
					std::find(names.begin(), names.end(), _a.key)
				);
			auto b_it=std::distance(
					std::begin(names),
					std::find(names.begin(), names.end(), _b.key)
				);

			return a_it < b_it;
			//TODO: We could configure a "name" order...
			//return _a.key < _b.key;
		}
	);

	current_index=0;
	exit_index=lookup.size();
	cancel_index=lookup.size()+1;
}

void properties::slumber(
	dfw::input& /*input*/,
	int
) {

	exchange_data.properties=nullptr;
	exchange_data.properties_blueprint=nullptr;
	exchange_data.edited_thing=nullptr;
	exchange_data.edited_thing_blueprint=nullptr;
	exchange_data.edited_poly=nullptr;
	exchange_data.edited_poly_blueprint=nullptr;
	property_manager=nullptr;
	blueprint=nullptr;

}

void properties::loop(
	dfw::input& _input,
	const dfw::loop_iteration_data& /*lid*/
) {

	if(_input().is_exit_signal()) {

		set_leave(true);
		return;
	}

	text_mode
		? input_text(_input)
		: input_traverse(_input);
}

void properties::input_traverse(dfw::input& _input) {

 	if(_input.is_input_down(input::escape)) {
		cancel_changes();
		pop_state();
		return;
	}

	//Next
	if(_input.is_input_down(input::down)) {

		//Allow wrap...
		if(current_index < cancel_index) {
			++current_index;
		}
		else {

			current_index=0;
		}
		return;
	}
	//Prev...
	else if(_input.is_input_down(input::up)) {

		if(current_index == 0) {

			current_index=cancel_index;
		}
		else {

			--current_index;
		}

		return;
	}

	if(_input.is_input_down(input::enter)) {

		if(current_index==cancel_index) {
			cancel_changes();
			pop_state();
			return;
		}

		if(current_index==exit_index) {
			save_changes();
			pop_state();
			return;
		}


		text_mode=true;
		current_value.clear();
		_input().start_text_input();

		auto is_numeric=[](const SDL_Event& _event) -> bool {
			try {

				const std::string str=_event.text.text;

				//Allows idiot stuff like -134-23 but hey..
				if(str=="-") {

					return true;
				}

				int val=std::stoi(str);
				return val >= 0 && val <= 9;
			}
			catch(std::invalid_argument&) {
				return false;
			}
		};

		//Set the filter...
		auto lookup_data=lookup.at(current_index);
		switch(lookup_data.type) {
			case option_types::str:

			break;
			case option_types::integer:
				_input().set_text_filter(is_numeric);
			break;
			case option_types::decimal:
				_input().set_text_filter([is_numeric](const SDL_Event& _event) -> bool {

					const std::string txt{_event.text.text}; //_event.text.text is const char *, with no ==
					if(txt==".") {
						return true;
					}

					return is_numeric(_event);
				});
			break;
		}
		return;
	}
}

void properties::input_text(dfw::input& _input) {

	auto done=[&_input, this]() {

		_input().clear_text_filter();
		_input().stop_text_input();
		_input().clear_text_input();

		//inflate the new properties so they are displayed.
		tile_editor::entity_inflator inflator;
		if(nullptr!=exchange_data.edited_thing) {

			inflator.inflate(*exchange_data.edited_thing, *exchange_data.edited_thing_blueprint);
		}
		else if(nullptr!=exchange_data.edited_poly) {

			inflator.inflate(*exchange_data.edited_poly, *exchange_data.edited_poly_blueprint);
		}

		current_value="";
		text_mode=false;
	};

	if(_input.is_input_down(input::escape)) {

		done();
		return;
	}

	if(_input.is_input_down(input::enter)) {

		auto lookup_data=lookup.at(current_index);
		switch(lookup_data.type) {
			case option_types::str:
				property_manager->string_properties.at(lookup_data.key)=current_value;
			break;
			case option_types::integer:
				property_manager->int_properties.at(lookup_data.key)=std::stoi(current_value);
			break;
			case option_types::decimal:
				property_manager->double_properties.at(lookup_data.key)=std::stod(current_value);
			break;
		}

		done();
		return;
	}

	if(_input.is_input_down(input::backspace)) {

		if(current_value.length()) {
			current_value.pop_back();
		}

		_input().clear_text_input();
		return;
	}

	if(_input().is_text_input()) {

		current_value+=_input().get_text_input();
		_input().clear_text_input();
	}
}

void properties::request_draw(
	dfw::controller_view_manager& _cvm
) {
	_cvm.add(state_editor);
	_cvm.add_ptr(this);
}


void properties::draw(ldv::screen& _screen, int /*fps*/) {

	properties::printer pr{current_index, cancel_index+1, current_value, text_mode, std::stringstream{}};

	if(current_index==exit_index) {
		pr.special("exit");
	}
	else if(current_index==cancel_index) {
		pr.special("cancel");
	}
	else {
		auto lookup_data=lookup.at(current_index);
		switch(lookup_data.type) {
			case option_types::str:
				pr.print(blueprint->string_properties.at(lookup_data.key), property_manager->string_properties.at(lookup_data.key), "string");
			break;
			case option_types::integer:
				pr.print(blueprint->int_properties.at(lookup_data.key), property_manager->int_properties.at(lookup_data.key), "int");
			break;
			case option_types::decimal:
				pr.print(blueprint->double_properties.at(lookup_data.key), property_manager->double_properties.at(lookup_data.key), "double");
			break;
		}
	}

	//Name...
	ldv::ttf_representation txt_menu{
		ttf_manager.get(tile_editor::definitions::main_font_name, tile_editor::definitions::main_font_size),
		ldv::rgba8(255, 255, 255, 255),
		"",
	};

	txt_menu.set_max_width(_screen.get_rect().w / 2);
	txt_menu.set_line_height_ratio(tile_editor::definitions::line_height_ratio);
	txt_menu.set_text(pr.ss.str());

	//centered alignment.
	ldv::representation_alignment center={
		ldv::representation_alignment::h::center,
		ldv::representation_alignment::v::center
	};

	//background box
	auto box=txt_menu.get_text_position();
	ldv::box_representation background(
		{
			{0, 0},
			box.w+tile_editor::definitions::property_box_padding,
			box.h+tile_editor::definitions::property_box_padding
		},
		ldv::rgba8(0,0,0,255)
	);
	background.align(_screen.get_rect(), center);
	background.draw(_screen);

	//border...
	background.set_filltype(ldv::polygon_representation::type::line);
	background.set_color(ldv::rgba8(255, 255, 255, 255));
	background.draw(_screen);

	//and finally the text.
	txt_menu.align(background, center);
	txt_menu.draw(_screen);
}

void properties::save_changes() {

	//Do nothing, actually :).
}

void properties::cancel_changes() {

	*(exchange_data.properties)=*property_manager;
}
