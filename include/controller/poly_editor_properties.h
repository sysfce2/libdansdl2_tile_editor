#pragma once

//local
#include "states.h"
#include "editor_types/poly_layer.h"
#include "app/exchange_data.h"

#include <dfw/controller_interface.h>
#include <ldtools/ttf_manager.h>
#include <lm/logger.h>
#include <tools/options_menu.h>
#include <map>

namespace controller {

//!Controller for the poly editor layer properties
class poly_editor_properties:
	public dfw::controller_interface {

	public:

								poly_editor_properties(lm::logger&, ldtools::ttf_manager&, tile_editor::exchange_data&);
	virtual void 				loop(dfw::input&, const dfw::loop_iteration_data&);
	virtual void 				draw(ldv::screen&, int);
	virtual void 				awake(dfw::input&, int=-1);
	virtual void 				slumber(dfw::input& /*input*/, int);
	virtual bool				can_leave_state() const {return true;}

	private:

	enum windings {any=0, clockwise, counterclockwise};

	void                        input_traverse(dfw::input&);
	void                        input_text(dfw::input&);
	void                        save_changes();
	int                         winding_to_int(tile_editor::poly_layer::windings) const;
	tile_editor::poly_layer::windings int_to_winding(int) const;

	//references...
	lm::logger&                 log;
	ldtools::ttf_manager&       ttf_manager;
	tile_editor::exchange_data& exchange_data;

	enum menu_types{
		menu_layer_id,
		menu_layer_alpha,
		menu_layer_set,
		menu_layer_gridset,
		menu_layer_winding,
		menu_layer_exit,
		menu_layer_cancel,
		menu_layer_end
	};

	//properties
	tile_editor::poly_layer*   layer{nullptr};
	tools::options_menu<int>    menu;
	int                         current_key{0};
	std::map<int, std::string>  sets,
	                            gridsets;
	bool                        text_mode{false};
	std::string                 input_value;
};

}
