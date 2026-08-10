#include "controller/thing_editor_properties.h"
#include "input/input.h"
#include "blueprint_types/map_blueprint.h"
#include "app/definitions.h"

#include <ldv/ttf_representation.h>

#include <vector>
#include <sstream>

using namespace controller;

thing_editor_properties::thing_editor_properties(
	lm::logger& _log,
	ldtools::ttf_manager& _ttf_manager,
	tile_editor::exchange_data& _exchange_data
):log(_log),
	ttf_manager{_ttf_manager},
	exchange_data{_exchange_data} {

	std::vector<int> intsets{};

	menu.set_wrap(true);
	menu.insert(menu_layer_id, "");
	menu.insert(menu_layer_alpha, 0, 0, 255, false);
	menu.insert(menu_layer_set, intsets, true);
	menu.insert(menu_layer_gridset, intsets, true);
	menu.insert(menu_layer_exit);
	menu.insert(menu_layer_cancel);
}

void thing_editor_properties::awake(
	dfw::input&,
	int
) {

	if(!exchange_data.has(state_thing_editor_properties)) {

		throw std::runtime_error("could not find data for thing_editor_properties");
	}

	exchange_data.recover(state_thing_editor_properties);
	layer=static_cast<tile_editor::thing_layer*>(exchange_data.layer);
	exchange_data.layer=nullptr;

	sets.clear();
	menu.clear_choice(menu_layer_set);

	for(const auto& pair : exchange_data.blueprint->thingsets) {

		int id=pair.first;
		sets[id]=pair.second.name;
		menu.add(menu_layer_set, id);
	}

	gridsets.clear();
	menu.clear_choice(menu_layer_gridset);

	for(const auto& pair : exchange_data.blueprint->gridsets) {

		int id=pair.first;
		sets[id]=pair.second.name;
		menu.add(menu_layer_gridset, id);
	}

	menu.set(menu_layer_id, layer->id);
	menu.set(menu_layer_alpha, layer->alpha);
	menu.set(menu_layer_set, (int)layer->set);
	menu.set(menu_layer_gridset, (int)layer->gridset);
	current_key=0;
}

void thing_editor_properties::slumber(
	dfw::input&,
	int
) {

	layer=nullptr;
}

void thing_editor_properties::loop(
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

void thing_editor_properties::input_traverse(
	dfw::input& _input
) {

 	if(_input.is_input_down(input::escape)) {

		pop_state();
		return;
	}

	if(_input.is_input_down(input::down)) {
		current_key=menu.adjacent_key(current_key, decltype(menu)::browse_dir::next);
		return;
	}
	else if(_input.is_input_down(input::up)) {
		current_key=menu.adjacent_key(current_key, decltype(menu)::browse_dir::previous);
		return;
	}

	if(_input.is_input_down(input::enter)) {

		switch(current_key) {

			case menu_layer_cancel:
				pop_state();
				return;

			case menu_layer_exit:
				save_changes();
				pop_state();
				return;

			case menu_layer_set:
				menu.browse(menu_layer_set, decltype(menu)::browse_dir::next);
				return;

			case menu_layer_gridset:
				menu.browse(menu_layer_gridset, decltype(menu)::browse_dir::next);
				return;

			case menu_layer_alpha:
				text_mode=true;
				input_value.clear();
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
				text_mode=true;
				input_value.clear();
				_input().start_text_input();
			break;
		}
	}
}

void thing_editor_properties::input_text(dfw::input& _input) {

	auto done=[&_input, this]() {

		_input().clear_text_filter();
		_input().stop_text_input();
		_input().clear_text_input();
		input_value="";
		text_mode=false;
	};

	if(_input.is_input_down(input::escape)) {

		done();
		return;
	}

	if(_input.is_input_down(input::enter)) {

		switch(current_key) {
			case menu_layer_id:
				menu.set(menu_layer_id, input_value);
			break;
			case menu_layer_alpha:
			{
				int val=std::stoi(input_value);
				if(val >= 0 && val <= 255) {
					menu.set(menu_layer_alpha, val);
				}
			}
			break;
		}

		done();
		return;
	}

	if(_input.is_input_down(input::backspace)) {

		if(input_value.length()) {
			input_value.pop_back();
		}

		_input().clear_text_input();
		return;
	}

	if(_input().is_text_input()) {

		input_value+=_input().get_text_input();
		_input().clear_text_input();
	}
}

void thing_editor_properties::draw(
	ldv::screen& _screen,
	int /*fps*/
) {

	_screen.clear(ldv::rgba8(0, 0, 0, 255));

	auto is_current=[this](int _index) -> bool {
		return _index==current_key;
	};

	auto selected=[is_current](int _index) -> const char *{

		return is_current(_index) ? "[>] " : "[ ] ";
	};

	std::stringstream ss;
	for(int i=0; i < menu_layer_end; i++) {

		switch(i) {
			case menu_layer_id:
				if(is_current(i) && text_mode) {
					ss<<"[*] id (string): "<<input_value<<std::endl;
				}
				else {
					ss<<selected(i)<<"id (string): "<<menu.get_string(i)<<std::endl;
				}
			break;
			case menu_layer_alpha:
				if(is_current(i) && text_mode) {
					ss<<"[*] alpha (0-255): "<<input_value<<std::endl;
				}
				else {
					ss<<selected(i)<<"alpha (0-255): "<<menu.get_int(i)<<std::endl;
				}
			break;
			case menu_layer_set:
				ss<<selected(i)<<"set (choice): "<<sets[menu.get_int(i)]<<std::endl;
			break;
			case menu_layer_gridset:
				ss<<selected(i)<<"gridset (choice): "<<sets[menu.get_int(i)]<<std::endl;
			break;
			case menu_layer_exit:
				ss<<selected(i)<<"back"<<std::endl;
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

	txt_menu.go_to({0, 0});
	txt_menu.draw(_screen);
}

void thing_editor_properties::save_changes() {

	layer->id=menu.get_string(menu_layer_id);
	layer->alpha=menu.get_int(menu_layer_alpha);
	layer->set=(std::size_t)menu.get_int(menu_layer_set);
	layer->gridset=(std::size_t)menu.get_int(menu_layer_gridset);
}
