#pragma once

//local
#include "states.h"
#include "app/exchange_data.h"
#include "tools/message_manager.h"
#include "editor_types/map.h"


#include <dfw/controller_interface.h>
#include <lm/logger.h>
#include <ldv/representation.h>
#include <ldtools/ttf_manager.h>
#include <tools/options_menu.h>

#include <cmath>

namespace controller {

class layer_selector:
	public dfw::controller_interface {

	public:

								layer_selector(lm::logger&, ldtools::ttf_manager&, tools::message_manager&, tile_editor::exchange_data&);
	virtual void 				loop(dfw::input&, const dfw::loop_iteration_data&);
	virtual void 				draw(ldv::screen&, int);
	virtual void 				awake(dfw::input& /*input*/, int=-1);
	virtual void 				slumber(dfw::input& /*input*/, int);
	virtual bool				can_leave_state() const {return true;}
	void                        request_draw(dfw::controller_view_manager&);

	private:

	void                        input_traverse(dfw::input&);
	void                        input_new(dfw::input&);
	void                        input_new_text(dfw::input&);
	void                        input_new_traverse(dfw::input&);
	void                        insert_layer();
	void                        delete_layer();
	void                        draw_traverse(ldv::screen&);
	void                        draw_new(ldv::screen&);
	//!Draws the background layer, plus aligns the given representation with it.
	void                        draw_background(ldv::screen&, ldv::representation&);

	enum menu_types{
		menu_layer_type,
		menu_layer_id,
		menu_layer_alpha,
		menu_layer_ok,
		menu_layer_cancel,
		menu_layer_end
	};

	enum layer_types {
		tile=0,
		thing,
		poly
	};

	//references...
	lm::logger&                 log;
	ldtools::ttf_manager&       ttf_manager;
	tools::message_manager&     message_manager;
	tile_editor::exchange_data& exchange_data;

	//properties
	std::vector<tile_editor::map::layerptr> *   layers;
	tools::options_menu<int>                    new_layer_menu;
	int                                         new_layer_menu_key{0};
	bool                                        new_mode{false},
	                                            new_mode_text{false};
	std::string                                 new_mode_value;
};

}
