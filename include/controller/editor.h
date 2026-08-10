#pragma once

//local
#include "states.h"
#include "tools/message_manager.h"
#include "app/exchange_data.h"
#include "app/env.h"
#include "app/screen_titler.h"
#include "blueprint_types/map_blueprint.h"
#include "blueprint_types/grid_data.h"
#include "editor_types/map.h"
#include "editor_types/thing.h"
#include "editor_types/poly.h"


#include <ldv/ttf_representation.h>
#include <dfw/controller_interface.h>
#include <lm/logger.h>
#include <ldtools/ttf_manager.h>
#include <tools/grid_list.h>
#include <tools/vertical_list.h>

#include <functional>

namespace controller {

class editor:
	public dfw::controller_interface {

	public:

								editor(
									lm::logger&, 
									ldtools::ttf_manager&, 
									tools::message_manager&, 
									tile_editor::exchange_data&, 
									const tile_editor::env&, 
									tile_editor::screen_titler&, 
									unsigned int, 
									unsigned int
								);
	virtual void 				loop(dfw::input&, const dfw::loop_iteration_data&);
	virtual void 				draw(ldv::screen&, int);
	virtual void 				awake(dfw::input& /*input*/, int);
	virtual void 				slumber(dfw::input& /*input*/, int) {}
	virtual bool				can_leave_state() const {return true;}
	void                        load_session(const std::string&);
	void                        load_map(const std::string&);

	private:

	enum key_modifiers {
		key_modifier_none=0,
		key_modifier_lshift=1,
		key_modifier_lctrl=2,
		key_modifier_lalt=4
	};

	using editor_point=ldt::point_2d<int>;

	struct {
		editor_point        point{0,0};
		bool                engaged{false};
		struct {
			int min{0}, max{0};
		}                  rangex, rangey;
	} multiclick;

	//!returns the world position from the screen position (camera considered).
	editor_point                get_world_position(ldt::point_2d<int>) const;
	//!returns grid-based position (for tile layers!) from world position.
	editor_point                get_grid_position(ldt::point_2d<int>) const;
	editor_point                snap_to_grid(editor_point) const;
	void                        arrow_input_set(int, int, int);
	void                        page_input_set(int);
	void                        arrow_input_layer(int, int, int);
	void                        arrow_input_layer(tile_editor::tile_layer&, int, int, int);
	void                        arrow_input_layer(tile_editor::thing_layer&, int, int, int);
	void                        arrow_input_layer(tile_editor::poly_layer&, int, int, int);
	void                        click_input(int, int);
	void                        left_click_input(int, tile_editor::tile_layer&);
	void                        right_click_input(int, tile_editor::tile_layer&);
	void                        left_click_input(int, tile_editor::thing_layer&);
	void                        right_click_input(int, tile_editor::thing_layer&);
	void                        left_click_input(int, tile_editor::poly_layer&);
	void                        right_click_input(int, tile_editor::poly_layer&);
	void                        del_input();
	void                        del_input(tile_editor::thing_layer&);
	void                        del_input(tile_editor::poly_layer&);
	void                        subgrid_input(bool);
	void                        draw_messages(ldv::screen&);
	void                        draw_hud(ldv::screen&);
	void                        draw_hud_thing_info(std::stringstream&, const tile_editor::thing&, const tile_editor::thing_definition&);
	void                        draw_hud_poly_info(std::stringstream&, const tile_editor::poly&, const tile_editor::poly_definition&);

	void                        draw_grid(ldv::screen&);
	void                        draw_layers(ldv::screen&);
	void                        draw_layer(ldv::screen&, const tile_editor::tile_layer&);
	void                        draw_layer(ldv::screen&, const tile_editor::thing_layer&);
	void                        draw_layer(ldv::screen&, const tile_editor::poly_layer&);
	void                        draw_set(ldv::screen&);
	//!draws the background box for the toolset. also returns its x.
	int                         draw_set_background(ldv::screen&);
	//!draws the background box for a tile in the toolset.
	void                        draw_tile_background(ldv::screen&, int, int, unsigned int, unsigned int);
	//!draws a text item for the toolset.
	void                        draw_set_text(ldv::screen& _screen, int _x, int _y, int _h, tile_editor::color _color, const std::string& _name, bool _is_current);
	void                        draw_set(ldv::screen&, const tile_editor::tile_layer&);
	void                        draw_set(ldv::screen&, const tile_editor::thing_layer&);
	void                        draw_set(ldv::screen&, const tile_editor::poly_layer&);
	void                        draw_cursor(ldv::screen&);
	void                        zoom_in();
	void                        zoom_out();
	void                        make_subgrid_smaller();
	void                        make_subgrid_larger();
	void                        next_layer();
	void                        move_camera(int, int);
	void                        previous_layer();
	//!resets all layer sensitive data (selections, polygons in the making...).
	void                        layer_change_cleanup();
	void                        toggle_layer_draw_mode();
	void                        load_layer_toolset();
	void                        save_current();
	void                        receive_message(tools::message_manager::notify_event_type);
	void                        open_layer_settings();
	void                        open_map_properties();
	void                        open_layer_selector();
	void                        open_thing_properties(tile_editor::property_manager& _properties, tile_editor::thing_definition& _blueprint);
	void                        open_poly_properties(tile_editor::property_manager& _properties, tile_editor::poly_definition& _blueprint);
	//!Helpers for layer dispatchers, will do nothing if there are no layers, saving us 100 checks.
	bool                        dispatch_layer(tile_editor::const_layer_visitor&);
	bool                        dispatch_layer(tile_editor::layer_visitor&);
	int                         blend_alpha(int, int) const;
	void                        toggle_set_gui();
	void                        close_current_poly(tile_editor::poly_layer&);
	const tile_editor::grid_data& get_grid_data_for_layer_index(std::size_t) const;
	const tile_editor::grid_data& get_grid_data_for_layer(const tile_editor::layer&) const;

	//references...
	lm::logger&                 log;
	ldtools::ttf_manager&       ttf_manager;
	tools::message_manager&     message_manager;
	tile_editor::exchange_data& exchange_data;
	tile_editor::screen_titler& screen_titler;

	//TODO: These should likely go somewhere else...
	ldv::texture                cursor_tex;
	ldtools::sprite_table       cursor_table;
	tile_editor::map_blueprint  session;
	tile_editor::map            map;
	std::map<std::string, std::unique_ptr<ldv::texture>> tileset_textures;

	enum class layer_draw_modes {all, current, stack} layer_draw_mode{layer_draw_modes::stack};
	ldv::rect                   screen_rect;
	ldv::camera                 camera;
	ldv::ttf_representation     last_message_rep;
	ldt::point_2d<int>          mouse_pos;

	struct frame {
		std::size_t             id;
		ldtools::sprite_frame   frame;
	};

	//The sprite_frame does not store its id, so the grid will store the
	//natural pair id-sprite.
	using tilelist_t=tools::grid_list<ldtools::sprite_table::container::value_type>;
	using thinglist_t=tools::vertical_list<tile_editor::thing_definition>;
	using polylist_t=tools::vertical_list<tile_editor::poly_definition>;

	tilelist_t                  tile_list;
	thinglist_t                 thing_list;
	polylist_t                  poly_list;
	std::string                 current_filename,
	                            current_session_filename;
	std::size_t	                current_layer{0},
	                            subgrid_factor{0};
	tile_editor::thing *        selected_thing{nullptr};
	tile_editor::poly *         selected_poly{nullptr};
	//!Function that returns the origin of a "thing" based on the current "thing_center" attribute.
	//!Such origin is what appears on the thing crosshair.
	std::function<editor_point(int, int, int, int)>	thing_origin_fn;
	//!Functions that returns a thing box based on the current "thing_center" attribute.
	//!This box is on cartesian format, with its origin at bottom left.
	std::function<ldt::box<int, unsigned>(editor_point, unsigned, unsigned)> thing_box_fn;
	bool                        show_set{true},
	                            tile_delete_mode{false};
	std::vector<tile_editor::poly_point> current_poly_vertices;

	static const int            grid_list_w{32},
	                            grid_list_h{32},
	                            grid_list_margin{8},
	                            vertical_list_h{32},
	                            vertical_list_margin{8};
};
}

