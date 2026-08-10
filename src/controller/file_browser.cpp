#include "controller/file_browser.h"
#include "app/definitions.h"
#include "input/input.h"

#include <tools/json.h>
#include <tools/file_utils.h>
#include <tools/string_utils.h>
#include <ldv/ttf_representation.h>
#include <lm/log.h>

#include <algorithm>

#ifndef WITH_FILESYSTEM
#include <cstdlib>
#endif

using namespace controller;

file_browser::file_browser(
	lm::logger& plog,
	ldtools::ttf_manager& _ttfman,
	tile_editor::exchange_data& _exchange_data,
	const tile_editor::env& _env,
	int _window_height
):
log(plog),
ttf_manager{_ttfman},
exchange_data{_exchange_data},
mode{working_modes::navigate},
current_directory{tools::filesystem::current_path()},
pager{0, 0} {

	//Mount layout...
	layout.map_font("default_font", ttf_manager.get(tile_editor::definitions::main_font_name, tile_editor::definitions::main_font_size));

	auto root=tools::parse_json_string(
		tools::dump_file(
			_env.build_data_path("layouts.json")
		)
	);

	layout.parse(root["file_browser"]);
	first_selection_y=layout.get_int("first_selection_y");
	y_selection_factor=layout.get_int("y_selection_factor");

	//Calculate items per page based on window height...
	int excess_height=_window_height-first_selection_y;
	pager.set_items_per_page(excess_height / y_selection_factor);

	//Setup data...
	title="file browser";
	extract_entries();
	refresh_list_view();
	position_selector();
	compose_title();
}

void file_browser::awake(
	dfw::input& /*_input*/,
	int
) {

	lm::log(log).info()<<"file browser controller awakens from invoker "<<exchange_data.file_browser_invoker_id<<"..."<<std::endl;

	//On awake there must always be something for this controller.
	exchange_data.recover(state_file_browser);

	if(exchange_data.file_browser_allow_create != allow_create) {

		allow_create=exchange_data.file_browser_allow_create;
		lm::log(log).info()<<"file browser changes allow_create to "<<allow_create<<std::endl;

		extract_entries();
		refresh_list_view();
		position_selector();
	}

	title=exchange_data.file_browser_title;
	invoker_id=exchange_data.file_browser_invoker_id;

	compose_title();
}

void file_browser::solve(bool _result, const std::string& _choice) {

	exchange_data.put(invoker_id);
	exchange_data.file_browser_success=_result;
	exchange_data.file_browser_choice=_choice;

	lm::log(log).info()<<"file browser solves to ["
		<<exchange_data.file_browser_success
		<<", "<<exchange_data.file_browser_choice
		<<"] back to "<<invoker_id<<std::endl;

	pop_state();
}


void file_browser::loop(dfw::input& _input, const dfw::loop_iteration_data& /*lid*/) {

	if(_input().is_exit_signal()) {

		set_leave(true);
		return;
	}

/*
//TODO:
	if(_input.is_input_down(input::help)) {

		push_state(state_help);
	}
*/

	switch(mode) {
		case working_modes::navigate:
			input_navigation(_input);
		break;
		case working_modes::create:
			input_create(_input);
		break;
	}
}

void file_browser::draw(ldv::screen& screen, int /*fps*/) {

	screen.clear(ldv::rgba8(0, 0, 0, 255));
	layout.draw(screen);
}

void file_browser::extract_entries() {

	contents.clear();

	if(allow_create) {

		contents.push_back({
			"new",
			entry::entry_type::create
		});
	}

	if(current_directory!=current_directory.parent_path()) {
		contents.push_back({
			"..",
			entry::entry_type::dir
		});
	}

	for(const auto& dir_entry : tools::filesystem::directory_iterator(current_directory)) {

		const auto& path=dir_entry.path();
		std::string filename=path.filename();

		if(tools::filesystem::is_directory(path)) {

			contents.push_back({
				filename,
				entry::entry_type::dir
			});
		}
		else if(tools::filesystem::is_regular_file(path)){

			contents.push_back({
				filename,
				entry::entry_type::file
			});
		}
	}

	std::sort(
		std::begin(contents),
		std::end(contents)
	);

	pager.set_item_count(contents.size());
	pager.reset();
}

void file_browser::refresh_list_view() {

	std::string files;
	long int ipp=pager.get_items_per_page();
	auto it=std::begin(contents)+(pager.get_current_page() * ipp);
	auto begin=it;

	//Read the next N items...
	for(; it!=std::end(contents) && std::distance(begin, it) < ipp; ++it) {

		files+=it->is_dir()
			? "["+it->path_name+"]\n"
			: it->is_new()
				? "("+it->path_name+")\n"
				: it->path_name+"\n";
	}

	static_cast<ldv::ttf_representation *>(
		layout.get_by_id("list")
	)->set_text(files);
}

void file_browser::position_selector() {

	int y_pos=first_selection_y+(pager.get_relative_index()*y_selection_factor);

	static_cast<ldv::ttf_representation *>(
		layout.get_by_id("selection")
	)->go_to({0, y_pos});
}

void file_browser::compose_title() {

	std::string final_title{title};

	final_title+=" : "
		+current_directory.string()
		+" ["
		+std::to_string(pager.get_current_page()+1)
		+"/"
		+std::to_string(pager.get_pages_count())
		+"] ipp: "
		+std::to_string(pager.get_items_per_page());

	static_cast<ldv::ttf_representation *>(
		layout.get_by_id("title")
	)->set_text(final_title);
}

void file_browser::input_navigation(dfw::input& _input) {

	bool check_change=false;

	//Cancelling...
	if(_input.is_input_down(input::escape)) {

		return solve(false, "");
	}

	//Movement...
	if(_input.is_input_down(input::down)) {

		pager.cycle_item(decltype(pager)::dir::next);
		check_change=true;
	}
	else if(_input.is_input_down(input::up)) {

		pager.cycle_item(decltype(pager)::dir::previous);
		check_change=true;
	}
	else if(_input.is_input_down(input::pagedown)
		|| _input.is_input_down(input::right)
	) {

		pager.turn_page(decltype(pager)::dir::next);
		check_change=true;
	}
	else if(_input.is_input_down(input::pageup)
		|| _input.is_input_down(input::left)
	) {

		pager.turn_page(decltype(pager)::dir::previous);
		check_change=true;
	}

	if(check_change) {

		if(pager.is_item_cycled()) {
			position_selector();
		}

		if(pager.is_page_turned()) {
			refresh_list_view();
			compose_title();
		}

		return;
	}

	if(_input.is_input_down(input::enter)) {

		const auto item=contents[pager.get_current_index()];

		if(item.is_dir()) {

			current_directory/={item.path_name};
#ifdef WITH_FILESYSTEM
			current_directory=current_directory.lexically_normal();
#else
		//with gcc7.5 we don't have lexically_normal, so we hack something
		//together and look the other way...
			char buffer[PATH_MAX];
			char * rpath=realpath(current_directory.c_str(), buffer);
			if(nullptr==rpath) {

				throw std::runtime_error("unable to normalize path");
			}

			current_directory=rpath;
#endif

			extract_entries();
			refresh_list_view();
			position_selector();
			compose_title();
		}
		else if(item.is_new()) {

			mode=working_modes::create;
			contents.front().path_name="";
			refresh_list_view();

			//TODO: Why is this not a parameter to start_text_input???
			_input().set_keydown_control_text_filter(true);
			_input().start_text_input();
			return;
		}
		else {

			auto final_path=current_directory/item.path_name;
			return solve(true, final_path.string());
		}
	}
}

void file_browser::input_create(dfw::input& _input) {

	if(_input.is_input_down(input::escape)) {

		mode=working_modes::navigate;
		_input().stop_text_input();
		_input().clear_text_input();

		//Restore the first item...
		contents.front().path_name="new";
		refresh_list_view();
		return;
	}

	if(_input().is_event_text()) {

		contents.front().path_name=_input().get_text_input();
		refresh_list_view();
	}

	if(_input.is_input_down(input::enter)) {

		if(!_input().get_text_input().size()) {
			return;
		}

		auto path=current_directory;
		path/=tools::str_trim({_input().get_text_input()});

		_input().stop_text_input();
		_input().clear_text_input();

		return solve(true, path.string());
	}
}
