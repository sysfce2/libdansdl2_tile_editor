#include "controller/layer_selector.h"
#include "input/input.h"
#include "app/definitions.h"
#include "editor_types/tile_layer.h"
#include "editor_types/thing_layer.h"
#include "editor_types/poly_layer.h"
#include "blueprint_types/map_blueprint.h"

#include <ldv/ttf_representation.h>
#include <ldv/box_representation.h>

#include <sstream>

using namespace controller;

layer_selector::layer_selector(
	lm::logger& plog,
	ldtools::ttf_manager& _ttf_manager,
	tools::message_manager& _message_manager,
	tile_editor::exchange_data& _exchange_data
)
	:log(plog),
	ttf_manager{_ttf_manager},
	message_manager{_message_manager},
	exchange_data{_exchange_data}{

	new_layer_menu.set_wrap(true);
	std::vector<int> types{tile, thing, poly};

	new_layer_menu.insert(menu_layer_type, types, true);
	new_layer_menu.insert(menu_layer_id, "default");
	new_layer_menu.insert(menu_layer_alpha, 0, 0, 255, false);
	new_layer_menu.insert(menu_layer_ok);
	new_layer_menu.insert(menu_layer_cancel);

}

void layer_selector::awake(
	dfw::input& /*input*/,
	int
) {

	if(!exchange_data.has(state_layer_selector)) {

		throw std::runtime_error("could not find data for layer_selector");
	}

	exchange_data.recover(state_layer_selector);
	layers=&exchange_data.map->layers;

	//reset the menu to its defaults...
	new_layer_menu.set(menu_layer_type, 0);
	new_layer_menu.set(menu_layer_id, "default");
	new_layer_menu.set(menu_layer_alpha, 255);
	new_layer_menu_key=menu_layer_type;
}

void layer_selector::slumber(
	dfw::input& /*input*/,
	int
) {

	layers=nullptr;
	exchange_data.map=nullptr;
	exchange_data.current_layer=nullptr;
	exchange_data.blueprint=nullptr;
}

void layer_selector::loop(
	dfw::input& _input,
	const dfw::loop_iteration_data& /*lid*/
) {

	if(_input().is_exit_signal()) {
		set_leave(true);
		return;
	}

	new_mode
		? input_new(_input)
		: input_traverse(_input);
}

void layer_selector::input_traverse(
	dfw::input& _input
) {
	if(_input.is_input_down(input::escape)
		|| _input.is_input_down(input::enter)) {

		pop_state();
		return;
	}

	//Layer selection...
	if(_input.is_input_down(input::up)) {

		if(*(exchange_data.current_layer) > 0) {
			--(*exchange_data.current_layer);
		}
		return;
	}
	else if(_input.is_input_down(input::down)) {

		if(*(exchange_data.current_layer) < layers->size()-1) {
			++(*exchange_data.current_layer);
		}
		return;
	}

	//Layer reordering...
	if(_input.is_input_down(input::pageup)) {

		if(*(exchange_data.current_layer) > 0) {
			std::swap( (*layers)[*(exchange_data.current_layer)], (*layers)[*(exchange_data.current_layer)-1]);
			--(*exchange_data.current_layer);
		}
		return;
	}
	else if(_input.is_input_down(input::pagedown)) {

		if(*(exchange_data.current_layer) < layers->size()-1) {
			std::swap( (*layers)[*(exchange_data.current_layer)], (*layers)[*(exchange_data.current_layer)+1]);
			++(*exchange_data.current_layer);
		}
		return;
	}

	if(_input.is_input_down(input::del)) {

		delete_layer();
		return;
	}

	if(_input.is_input_down(input::insert)) {

		new_mode=true;
		return;
	}
}

void layer_selector::input_new(
	dfw::input& _input
) {
	if(_input.is_input_down(input::escape)) {
		new_mode=false;
		return;
	}

	new_mode_text
		? input_new_text(_input)
		: input_new_traverse(_input);
}

void layer_selector::input_new_traverse(
	dfw::input& _input
) {
	//Layer selection...
	if(_input.is_input_down(input::up)) {

		if(new_layer_menu_key) {
			--new_layer_menu_key;
		}
		return;
	}
	else if(_input.is_input_down(input::down)) {

		if(new_layer_menu_key < menu_layer_end-1) {
			++new_layer_menu_key;
		}
		return;
	}

	if(_input.is_input_down(input::enter)) {

		switch(new_layer_menu_key) {
			case menu_layer_cancel:
				pop_state();
				return;

			case menu_layer_ok:
				insert_layer();
				new_mode=false;
				new_mode_text=false;
				new_mode_value="";
				return;

			case menu_layer_type:
				new_layer_menu.browse(menu_layer_type, decltype(new_layer_menu)::browse_dir::next);
				return;

			case menu_layer_alpha:
				new_mode_text=true;
				new_mode_value.clear();
				_input().start_text_input();
				_input().set_text_filter([](const SDL_Event& _event) -> bool {
					try {
						int val=std::stoi(_event.text.text);
						return val >= 0 && val <= 9;
					}
					catch(std::invalid_argument&) {
						return false;
					}
				});
			break;
			case menu_layer_id:
				new_mode_text=true;
				new_mode_value.clear();
				_input().start_text_input();
			break;
		}

		return;
	}
}

void layer_selector::input_new_text(
	dfw::input& _input
) {

	//TODO: this appears duplicated in many places. Perhaps we could extract
	//it to some specialized tool.

	auto done=[&_input, this]() {

		_input().clear_text_filter();
		_input().stop_text_input();
		_input().clear_text_input();
		new_mode_value="";
		new_mode_text=false;
	};

	if(_input.is_input_down(input::escape)) {

		done();
		return;
	}

	if(_input.is_input_down(input::enter)) {

		switch(new_layer_menu_key) {
			case menu_layer_id:
				new_layer_menu.set(menu_layer_id, new_mode_value);
			break;
			case menu_layer_alpha:
			{
				int val=std::stoi(new_mode_value);
				if(val >= 0 && val <= 255) {
					new_layer_menu.set(menu_layer_alpha, val);
				}
			}
			break;
		}

		done();
		return;
	}

	if(_input.is_input_down(input::backspace)) {

		if(new_mode_value.length()) {
			new_mode_value.pop_back();
		}

		_input().clear_text_input();
		return;
	}

	if(_input().is_text_input()) {

		new_mode_value+=_input().get_text_input();
		_input().clear_text_input();
	}
}

void layer_selector::request_draw(
	dfw::controller_view_manager& _cvm
) {
	_cvm.add(state_editor);
	_cvm.add_ptr(this);
}

void layer_selector::draw(
	ldv::screen& _screen,
	int /*fps*/
) {
	new_mode
		? draw_new(_screen)
		: draw_traverse(_screen);
}

void layer_selector::draw_traverse(
	ldv::screen& _screen
) {
	//add text...
	std::stringstream ss;

	if(!layers->size()) {

		ss<<"no layers, press insert/i to create"<<std::endl;
	}
	else {
		std::size_t index=0;
		for(const auto& layer : *layers) {

			ss<<(index++==(*exchange_data.current_layer) ? "[*] " : "[ ] ")
				<<layer->id<<" ["<<layer->alpha<<"]"<<std::endl;
		}
	}

	//draw...
	ldv::ttf_representation txt_menu{
		ttf_manager.get(tile_editor::definitions::main_font_name, tile_editor::definitions::main_font_size),
		ldv::rgba8(255, 255, 255, 255),
		ss.str()
	};

	txt_menu.set_line_height_ratio(tile_editor::definitions::line_height_ratio);

	draw_background(_screen, txt_menu);
	txt_menu.draw(_screen);
}

void layer_selector::draw_new(
	ldv::screen& _screen
) {

	auto is_current=[this](int _index) -> bool {
		return _index==new_layer_menu_key;
	};

	auto selected=[is_current](int _index) -> const char *{

		return is_current(_index) ? "[>] " : "[ ] ";
	};

	auto translate=[](int _type) -> const char * {

		switch(_type) {

			case tile: return "tile";
			case thing: return "thing";
			case poly: return "poly";
		}

		return "";
	};

	std::stringstream ss;
	for(int i=0; i < menu_layer_end; i++) {

		switch(i) {
			case menu_layer_type:
				ss<<selected(i)<<"type (choice): "<<translate(new_layer_menu.get_int(menu_layer_type))<<std::endl;
			break;
			case menu_layer_id:
				if(is_current(i) && new_mode_text) {
					ss<<"[*] id (string): "<<new_mode_value<<std::endl;
				}
				else {
					ss<<selected(i)<<"id (string): "<<new_layer_menu.get_string(i)<<std::endl;
				}
			break;
			case menu_layer_alpha:
				if(is_current(i) && new_mode_text) {
					ss<<"[*] alpha (0-255): "<<new_mode_value<<std::endl;
				}
				else {
					ss<<selected(i)<<"alpha (0-255): "<<new_layer_menu.get_int(i)<<std::endl;
				}
			break;
			case menu_layer_ok:
				ss<<selected(i)<<"create"<<std::endl;
			break;
			case menu_layer_cancel:
				ss<<selected(i)<<"cancel"<<std::endl;
			break;
		}
	}

	//Name...
	ldv::ttf_representation txt_menu{
		ttf_manager.get(tile_editor::definitions::main_font_name, tile_editor::definitions::main_font_size),
		ldv::rgba8(255, 255, 255, 255),
		ss.str()
	};

	txt_menu.set_line_height_ratio(tile_editor::definitions::line_height_ratio);

	draw_background(_screen, txt_menu);
	txt_menu.draw(_screen);
}

void layer_selector::draw_background(
	ldv::screen& _screen,
	ldv::representation& _rep
) {

//centered alignment.
	ldv::representation_alignment center={
		ldv::representation_alignment::h::center,
		ldv::representation_alignment::v::center
	};

	//background box
	auto box=_rep.get_view_position();
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

	//Align...
	_rep.align(background, center);
}

void layer_selector::delete_layer() {

	layers->erase(std::begin(*layers)+(*exchange_data.current_layer));
	if(*exchange_data.current_layer) {
		--(*exchange_data.current_layer);
	}

	if(!layers->size()) {

		message_manager.add("all layers deleted, returing to editor mode");
		pop_state();
		return;
	}

	message_manager.add("layer deleted");
}

void layer_selector::insert_layer() {

	auto alpha=new_layer_menu.get_int(menu_layer_alpha);
	auto id=new_layer_menu.get_string(menu_layer_id);

	tile_editor::layer * layer{nullptr};

	switch(new_layer_menu.get_int(menu_layer_type)) {

		case tile:
			layer=new tile_editor::tile_layer{
				std::begin(exchange_data.blueprint->tilesets)->first,
				std::begin(exchange_data.blueprint->gridsets)->first,
				alpha,
				id,
				{}
			};
		break;
		case thing:
			layer=new tile_editor::thing_layer{
				std::begin(exchange_data.blueprint->thingsets)->first,
				std::begin(exchange_data.blueprint->gridsets)->first,
				alpha,
				id,
				{}
			};
		break;
		case poly:
			layer=new tile_editor::poly_layer{
				std::begin(exchange_data.blueprint->polysets)->first,
				std::begin(exchange_data.blueprint->gridsets)->first,
				alpha,
				id,
				tile_editor::poly_layer::windings::clockwise,
				{}
			};
		break;
	}

	layers->insert(
		std::begin(*layers)+(*exchange_data.current_layer),
		std::unique_ptr<tile_editor::layer>{layer}
	);

	message_manager.add("new layer inserted with default set");
}
