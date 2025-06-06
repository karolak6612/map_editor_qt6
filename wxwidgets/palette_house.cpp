//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// Remere's Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Remere's Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#include "main.h"

#include "palette_house.h"

#include "settings.h"

#include "brush.h"
#include "editor.h"
#include "map.h"

#include "application.h"
#include "map_display.h"

#include "house_brush.h"
#include "house_exit_brush.h"
#include "spawn_brush.h"

// ============================================================================
// House palette

BEGIN_EVENT_TABLE(HousePalettePanel, PalettePanel)
EVT_TIMER(PALETTE_LAYOUT_FIX_TIMER, HousePalettePanel::OnLayoutFixTimer)

EVT_CHOICE(PALETTE_HOUSE_TOWN_CHOICE, HousePalettePanel::OnTownChange)

EVT_LISTBOX(PALETTE_HOUSE_LISTBOX, HousePalettePanel::OnListBoxChange)
EVT_LISTBOX_DCLICK(PALETTE_HOUSE_LISTBOX, HousePalettePanel::OnListBoxDoubleClick)
EVT_CONTEXT_MENU(HousePalettePanel::OnListBoxContextMenu)

EVT_BUTTON(PALETTE_HOUSE_ADD_HOUSE, HousePalettePanel::OnClickAddHouse)
EVT_BUTTON(PALETTE_HOUSE_EDIT_HOUSE, HousePalettePanel::OnClickEditHouse)
EVT_BUTTON(PALETTE_HOUSE_REMOVE_HOUSE, HousePalettePanel::OnClickRemoveHouse)

EVT_TOGGLEBUTTON(PALETTE_HOUSE_BRUSH_BUTTON, HousePalettePanel::OnClickHouseBrushButton)
EVT_TOGGLEBUTTON(PALETTE_HOUSE_SELECT_EXIT_BUTTON, HousePalettePanel::OnClickSelectExitButton)

EVT_MENU(PALETTE_HOUSE_CONTEXT_MOVE_TO_TOWN, HousePalettePanel::OnMoveHouseToTown)
END_EVENT_TABLE()

HousePalettePanel::HousePalettePanel(wxWindow* parent, wxWindowID id) :
	PalettePanel(parent, id),
	map(nullptr),
	do_resize_on_display(true),
	fix_size_timer(this, PALETTE_LAYOUT_FIX_TIMER) {
	wxSizer* topsizer = newd wxBoxSizer(wxVERTICAL);
	wxSizer* tmpsizer;

	wxSizer* sidesizer = newd wxStaticBoxSizer(wxVERTICAL, this, "Houses");
	town_choice = newd wxChoice(this, PALETTE_HOUSE_TOWN_CHOICE, wxDefaultPosition, wxDefaultSize, (int)0, (const wxString*)nullptr);
	sidesizer->Add(town_choice, 0, wxEXPAND);

	house_list = newd SortableListBox(this, PALETTE_HOUSE_LISTBOX, wxDefaultPosition, wxDefaultSize, 0, nullptr, wxLB_EXTENDED);
#ifdef __APPLE__
	// Used for detecting a deselect
	house_list->Bind(wxEVT_LEFT_UP, &HousePalettePanel::OnListBoxClick, this);
#endif
	// Bind context menu event to the list box
	house_list->Bind(wxEVT_CONTEXT_MENU, &HousePalettePanel::OnListBoxContextMenu, this);
	sidesizer->Add(house_list, 1, wxEXPAND);

	tmpsizer = newd wxBoxSizer(wxHORIZONTAL);
	wxSizerFlags sizerFlags(1);
	tmpsizer->Add(add_house_button = newd wxButton(this, PALETTE_HOUSE_ADD_HOUSE, "Add", wxDefaultPosition, wxSize(50, -1)), sizerFlags);
	tmpsizer->Add(edit_house_button = newd wxButton(this, PALETTE_HOUSE_EDIT_HOUSE, "Edit", wxDefaultPosition, wxSize(50, -1)), sizerFlags);
	tmpsizer->Add(remove_house_button = newd wxButton(this, PALETTE_HOUSE_REMOVE_HOUSE, "Remove", wxDefaultPosition, wxSize(70, -1)), sizerFlags);
	sidesizer->Add(tmpsizer, wxSizerFlags(0).Right());

	topsizer->Add(sidesizer, 1, wxEXPAND);

	// Temple position
	sidesizer = newd wxStaticBoxSizer(newd wxStaticBox(this, wxID_ANY, "Brushes", wxDefaultPosition, wxSize(150, 200)), wxVERTICAL);

	// sidesizer->Add(180, 1, wxEXPAND);

	tmpsizer = newd wxBoxSizer(wxHORIZONTAL);
	house_brush_button = newd wxToggleButton(this, PALETTE_HOUSE_BRUSH_BUTTON, "House tiles");
	tmpsizer->Add(house_brush_button);
	sidesizer->Add(tmpsizer, wxSizerFlags(1).Center());

	tmpsizer = newd wxBoxSizer(wxHORIZONTAL);
	select_position_button = newd wxToggleButton(this, PALETTE_HOUSE_SELECT_EXIT_BUTTON, "Select Exit");
	tmpsizer->Add(select_position_button);
	sidesizer->Add(tmpsizer, wxSizerFlags(1).Center());

	topsizer->Add(sidesizer, 0, wxEXPAND);

	SetSizerAndFit(topsizer);

	// Create context menu
	context_menu = newd wxMenu();
	context_menu->Append(PALETTE_HOUSE_CONTEXT_MOVE_TO_TOWN, "Move to Town...");
}

HousePalettePanel::~HousePalettePanel() {
	////
}

void HousePalettePanel::SetMap(Map* m) {
	g_gui.house_brush->setHouse(nullptr);
	map = m;
	OnUpdate();
}

void HousePalettePanel::OnSwitchIn() {
	PalettePanel::OnSwitchIn();
	// Extremely ugly hack to fix layout issue
	if (do_resize_on_display) {
		fix_size_timer.Start(100, true);
		do_resize_on_display = false;
	}
}

void HousePalettePanel::OnLayoutFixTimer(wxTimerEvent& WXUNUSED(event)) {
	wxWindow* w = this;
	while ((w = w->GetParent()) && dynamic_cast<PaletteWindow*>(w) == nullptr)
		;

	if (w) {
		w->SetSize(w->GetSize().GetWidth(), w->GetSize().GetHeight() + 1);
		w->SetSize(w->GetSize().GetWidth(), w->GetSize().GetHeight() - 1);
	}
}

void HousePalettePanel::SelectFirstBrush() {
	SelectHouseBrush();
}

Brush* HousePalettePanel::GetSelectedBrush() const {
	if (select_position_button->GetValue()) {
		House* house = GetCurrentlySelectedHouse();
		if (house) {
			g_gui.house_exit_brush->setHouse(house);
		}
		return (g_gui.house_exit_brush->getHouseID() != 0 ? g_gui.house_exit_brush : nullptr);
	} else if (house_brush_button->GetValue()) {
		g_gui.house_brush->setHouse(GetCurrentlySelectedHouse());
		return (g_gui.house_brush->getHouseID() != 0 ? g_gui.house_brush : nullptr);
	}
	return nullptr;
}

bool HousePalettePanel::SelectBrush(const Brush* whatbrush) {
	if (!whatbrush) {
		return false;
	}

	if (whatbrush->isHouse() && map) {
		const HouseBrush* house_brush = static_cast<const HouseBrush*>(whatbrush);
		for (HouseMap::iterator house_iter = map->houses.begin(); house_iter != map->houses.end(); ++house_iter) {
			if (house_iter->second->getID() == house_brush->getHouseID()) {
				for (uint32_t i = 0; i < town_choice->GetCount(); ++i) {
					Town* town = reinterpret_cast<Town*>(town_choice->GetClientData(i));
					// If it's "No Town" (nullptr) select it, or if it has the same town ID as the house
					if (town == nullptr || town->getID() == house_iter->second->townid) {
						SelectTown(i);
						for (uint32_t j = 0; j < house_list->GetCount(); ++j) {
							if (house_iter->second->getID() == reinterpret_cast<House*>(house_list->GetClientData(j))->getID()) {
								SelectHouse(j);
								return true;
							}
						}
						return true;
					}
				}
			}
		}
	} else if (whatbrush->isSpawn()) {
		SelectExitBrush();
	}
	return false;
}

int HousePalettePanel::GetSelectedBrushSize() const {
	return 0;
}

PaletteType HousePalettePanel::GetType() const {
	return TILESET_HOUSE;
}

void HousePalettePanel::SelectTown(size_t index) {
	ASSERT(town_choice->GetCount() >= index);

	if (map == nullptr || town_choice->GetCount() == 0) {
		// No towns :(
		add_house_button->Enable(false);
	} else {
		Town* what_town = reinterpret_cast<Town*>(town_choice->GetClientData(index));

		// Clear the old houselist
		house_list->Clear();

		for (HouseMap::iterator house_iter = map->houses.begin(); house_iter != map->houses.end(); ++house_iter) {
			if (what_town) {
				if (house_iter->second->townid == what_town->getID()) {
					house_list->Append(wxstr(house_iter->second->getDescription()), house_iter->second);
				}
			} else {
				// "No Town" selected!
				if (map->towns.getTown(house_iter->second->townid) == nullptr) {
					// The town doesn't exist
					house_list->Append(wxstr(house_iter->second->getDescription()), house_iter->second);
				}
			}
		}
		house_list->Sort();

		// Select first house
		SelectHouse(0);
		town_choice->SetSelection(index);
		add_house_button->Enable(what_town != nullptr);
		ASSERT(what_town == nullptr || add_house_button->IsEnabled() || !IsEnabled());
	}
}

void HousePalettePanel::SelectHouse(size_t index) {
	ASSERT(house_list->GetCount() >= index);

	if (house_list->GetCount() > 0) {
		edit_house_button->Enable(true);
		remove_house_button->Enable(true);
		select_position_button->Enable(true);
		house_brush_button->Enable(true);
		
		// Clear any existing selections first
		for (unsigned int i = 0; i < house_list->GetCount(); ++i) {
			house_list->Deselect(i);
		}
		
		// Select the house
		house_list->SetSelection(index);
		SelectHouseBrush();
	} else {
		// No houses :(
		edit_house_button->Enable(false);
		remove_house_button->Enable(false);
		select_position_button->Enable(false);
		house_brush_button->Enable(false);
	}

	SelectHouseBrush();
	g_gui.RefreshView();
}

House* HousePalettePanel::GetCurrentlySelectedHouse() const {
	wxArrayInt selections;
	if (house_list->GetCount() > 0 && house_list->GetSelections(selections) > 0) {
		// Return the first selected house (for brush operations)
		return reinterpret_cast<House*>(house_list->GetClientData(selections[0]));
	}
	return nullptr;
}

void HousePalettePanel::SelectHouseBrush() {
	if (house_list->GetCount() > 0) {
		house_brush_button->SetValue(true);
		select_position_button->SetValue(false);
	} else {
		house_brush_button->SetValue(false);
		select_position_button->SetValue(false);
	}
}

void HousePalettePanel::SelectExitBrush() {
	if (house_list->GetCount() > 0) {
		house_brush_button->SetValue(false);
		select_position_button->SetValue(true);
	}
}

void HousePalettePanel::OnUpdate() {
	int old_town_selection = town_choice->GetSelection();

	town_choice->Clear();
	house_list->Clear();

	if (map == nullptr) {
		return;
	}

	if (map->towns.count() != 0) {
		// Create choice control
		for (TownMap::iterator town_iter = map->towns.begin(); town_iter != map->towns.end(); ++town_iter) {
			town_choice->Append(wxstr(town_iter->second->getName()), town_iter->second);
		}
		town_choice->Append("No Town", (void*)(nullptr));
		if (old_town_selection <= 0) {
			SelectTown(0);
		} else if ((size_t)old_town_selection <= town_choice->GetCount()) {
			SelectTown(old_town_selection);
		} else {
			SelectTown(old_town_selection - 1);
		}

		house_list->Enable(true);
	} else {
		town_choice->Append("No Town", (void*)(nullptr));
		select_position_button->Enable(false);
		select_position_button->SetValue(false);
		house_brush_button->Enable(false);
		house_brush_button->SetValue(false);
		add_house_button->Enable(false);
		edit_house_button->Enable(false);
		remove_house_button->Enable(false);

		SelectTown(0);
	}
}

void HousePalettePanel::OnTownChange(wxCommandEvent& event) {
	SelectTown(event.GetSelection());
	g_gui.SelectBrush();
}

void HousePalettePanel::OnListBoxChange(wxCommandEvent& event) {
	// Check if there are multiple selections
	wxArrayInt selections;
	int count = house_list->GetSelections(selections);
	
	if (count == 1) {
		// Only one selection - handle it
		SelectHouse(event.GetSelection());
		g_gui.SelectBrush();
	} else if (count > 1) {
		// Multiple selections - enable/disable buttons appropriately
		edit_house_button->Enable(false); // Can only edit one house at a time
		remove_house_button->Enable(true);
		house_brush_button->Enable(true);
		select_position_button->Enable(true);
	}
}

void HousePalettePanel::OnListBoxDoubleClick(wxCommandEvent& event) {
	House* house = reinterpret_cast<House*>(event.GetClientData());
	// I find it extremly unlikely that one actually wants the exit at 0,0,0, so just treat it as the null value
	if (house && house->getExit() != Position(0, 0, 0)) {
		g_gui.SetScreenCenterPosition(house->getExit());
	}
}

void HousePalettePanel::OnListBoxContextMenu(wxContextMenuEvent& event) {
	if (map == nullptr || house_list->GetCount() == 0) {
		return;
	}

	// Only enable the menu if at least one house is selected
	wxArrayInt selections;
	if (house_list->GetSelections(selections) > 0) {
		// Get mouse position in screen coordinates
		wxPoint position = event.GetPosition();
		// If position is (-1, -1), this means the event was generated from the keyboard (e.g., Shift+F10)
		// In this case, get the position of the first selected item
		if (position == wxPoint(-1, -1)) {
			// Get the client rect of the first selected item
			wxRect rect;
			house_list->GetItemRect(selections[0], rect);
			// Convert to screen coordinates
			position = house_list->ClientToScreen(rect.GetPosition());
		}
		// Show context menu at the proper position
		PopupMenu(context_menu, house_list->ScreenToClient(position));
	}
}

void HousePalettePanel::OnMoveHouseToTown(wxCommandEvent& event) {
	if (map == nullptr || map->towns.count() == 0) {
		return;
	}

	// Get all selected houses
	wxArrayInt selections;
	int count = house_list->GetSelections(selections);
	
	if (count == 0) {
		return;
	}
	
	// Create a more informative title based on number of selected houses
	wxString title = count == 1 ? "Move House to Town" : wxString::Format("Move %d Houses to Town", count);

	// Create dialog to select town
	wxDialog* dialog = newd wxDialog(this, wxID_ANY, title, wxDefaultPosition, wxSize(220, 150));
	wxSizer* sizer = newd wxBoxSizer(wxVERTICAL);
	
	// Create choice control with towns
	wxChoice* town_list = newd wxChoice(dialog, wxID_ANY);
	for (TownMap::const_iterator town_iter = map->towns.begin(); town_iter != map->towns.end(); ++town_iter) {
		town_list->Append(wxstr(town_iter->second->getName()), town_iter->second);
	}
	
	if (town_list->GetCount() > 0) {
		town_list->SetSelection(0);
	}
	
	sizer->Add(newd wxStaticText(dialog, wxID_ANY, "Select destination town:"), 0, wxEXPAND | wxALL, 5);
	sizer->Add(town_list, 0, wxEXPAND | wxALL, 5);
	
	// Add OK/Cancel buttons
	wxSizer* button_sizer = newd wxBoxSizer(wxHORIZONTAL);
	button_sizer->Add(newd wxButton(dialog, wxID_OK, "OK"), wxSizerFlags(1).Center());
	button_sizer->Add(newd wxButton(dialog, wxID_CANCEL, "Cancel"), wxSizerFlags(1).Center());
	sizer->Add(button_sizer, 0, wxALIGN_CENTER | wxALL, 5);
	
	dialog->SetSizer(sizer);
	
	// Show dialog
	if (dialog->ShowModal() == wxID_OK) {
		if (town_list->GetSelection() != wxNOT_FOUND) {
			Town* town = reinterpret_cast<Town*>(town_list->GetClientData(town_list->GetSelection()));
			if (town) {
				// Change town for each selected house
				for (size_t i = 0; i < selections.GetCount(); ++i) {
					House* house = reinterpret_cast<House*>(house_list->GetClientData(selections[i]));
					if (house) {
						house->townid = town->getID();
					}
				}
				
				// Refresh the house list
				RefreshHouseList();
				
				// Refresh the map
				g_gui.RefreshView();
			}
		}
	}
	
	dialog->Destroy();
}

void HousePalettePanel::RefreshHouseList() {
	// Preserve current selections
	wxArrayInt selections;
	house_list->GetSelections(selections);
	std::vector<uint32_t> selected_house_ids;
	
	// Store IDs of all selected houses
	for (size_t i = 0; i < selections.GetCount(); ++i) {
		House* house = reinterpret_cast<House*>(house_list->GetClientData(selections[i]));
		if (house) {
			selected_house_ids.push_back(house->getID());
		}
	}
	
	// Reload the house list
	Town* what_town = reinterpret_cast<Town*>(town_choice->GetClientData(town_choice->GetSelection()));
	
	// Clear the old houselist
	house_list->Clear();
	
	for (HouseMap::iterator house_iter = map->houses.begin(); house_iter != map->houses.end(); ++house_iter) {
		if (what_town) {
			if (house_iter->second->townid == what_town->getID()) {
				house_list->Append(wxstr(house_iter->second->getDescription()), house_iter->second);
			}
		} else {
			// "No Town" selected!
			if (map->towns.getTown(house_iter->second->townid) == nullptr) {
				// The town doesn't exist
				house_list->Append(wxstr(house_iter->second->getDescription()), house_iter->second);
			}
		}
	}
	house_list->Sort();
	
	// Try to restore previous selections
	bool foundAny = false;
	for (unsigned int i = 0; i < house_list->GetCount(); ++i) {
		House* house = reinterpret_cast<House*>(house_list->GetClientData(i));
		if (house) {
			for (uint32_t selected_id : selected_house_ids) {
				if (house->getID() == selected_id) {
					house_list->SetSelection(i);
					foundAny = true;
					break;
				}
			}
		}
	}
	
	// If no selections could be restored, ensure buttons are in correct state
	if (!foundAny && house_list->GetCount() > 0) {
		SelectHouse(0);
	} else if (!foundAny) {
		edit_house_button->Enable(false);
		remove_house_button->Enable(false);
		select_position_button->Enable(false);
		house_brush_button->Enable(false);
	}
}

void HousePalettePanel::OnClickHouseBrushButton(wxCommandEvent& event) {
	SelectHouseBrush();
	g_gui.SelectBrush();
}

void HousePalettePanel::OnClickSelectExitButton(wxCommandEvent& event) {
	SelectExitBrush();
	g_gui.SelectBrush();
}

void HousePalettePanel::OnClickAddHouse(wxCommandEvent& event) {
	if (map == nullptr) {
		return;
	}

	House* new_house = newd House(*map);
	new_house->setID(map->houses.getEmptyID());

	std::ostringstream os;
	os << "Unnamed House #" << new_house->getID();
	new_house->name = os.str();
	Town* town = reinterpret_cast<Town*>(town_choice->GetClientData(town_choice->GetSelection()));

	ASSERT(town);
	new_house->townid = town->getID();

	map->houses.addHouse(new_house);
	house_list->Append(wxstr(new_house->getDescription()), new_house);
	SelectHouse(house_list->FindString(wxstr(new_house->getDescription())));
	g_gui.SelectBrush();
	refresh_timer.Start(300, true);
}

void HousePalettePanel::OnClickEditHouse(wxCommandEvent& event) {
	if (house_list->GetCount() == 0) {
		return;
	}
	if (map == nullptr) {
		return;
	}
	
	// Only edit if a single house is selected
	wxArrayInt selections;
	if (house_list->GetSelections(selections) != 1) {
		wxMessageBox("Please select only one house to edit.", "Edit House", wxOK | wxICON_INFORMATION);
		return;
	}
	
	int selection = selections[0];
	House* house = reinterpret_cast<House*>(house_list->GetClientData(selection));
	if (house) {
		wxDialog* d = newd EditHouseDialog(g_gui.root, map, house);
		int ret = d->ShowModal();
		if (ret == 1) {
			// Something changed, change name of house
			house_list->SetString(selection, wxstr(house->getDescription()));
			house_list->Sort();

			// refresh house list for town
			SelectTown(town_choice->GetSelection());
			g_gui.SelectBrush();
			refresh_timer.Start(300, true);
		}
	}
}

void HousePalettePanel::OnClickRemoveHouse(wxCommandEvent& event) {
	wxArrayInt selections;
	int count = house_list->GetSelections(selections);
	
	if (count == 0) {
		return;
	}
	
	// Ask for confirmation when removing multiple houses
	if (count > 1) {
		wxString message = wxString::Format("Are you sure you want to remove %d houses?", count);
		int response = wxMessageBox(message, "Confirm Removal", wxYES_NO | wxICON_QUESTION);
		if (response != wxYES) {
			return;
		}
	}
	
	// Sort selections in descending order to avoid index issues when deleting
	std::sort(selections.begin(), selections.end(), std::greater<int>());
	
	// Remove all selected houses
	for (size_t i = 0; i < selections.GetCount(); ++i) {
		int selection = selections[i];
		House* house = reinterpret_cast<House*>(house_list->GetClientData(selection));
		map->houses.removeHouse(house);
		house_list->Delete(selection);
	}
	
	refresh_timer.Start(300, true);
	
	// Select an appropriate remaining item if possible
	if (house_list->GetCount() > 0) {
		int new_selection = std::min(selections.back(), (int)house_list->GetCount() - 1);
		house_list->SetSelection(new_selection);
		edit_house_button->Enable(true);
		remove_house_button->Enable(true);
		select_position_button->Enable(true);
		house_brush_button->Enable(true);
	} else {
		select_position_button->Enable(false);
		select_position_button->SetValue(false);
		house_brush_button->Enable(false);
		house_brush_button->SetValue(false);
		edit_house_button->Enable(false);
		remove_house_button->Enable(false);
	}
	
	g_gui.SelectBrush();
	g_gui.RefreshView();
}

#ifdef __APPLE__
// On wxMac it is possible to deselect a wxListBox. (Unlike on the other platforms)
// EVT_LISTBOX is not triggered when the deselection is happening. http://trac.wxwidgets.org/ticket/15603
// Here we find out if the listbox was deselected using a normal mouse up event so we know when to disable the buttons and brushes.
void HousePalettePanel::OnListBoxClick(wxMouseEvent& event) {
	if (house_list->GetSelection() == wxNOT_FOUND) {
		select_position_button->Enable(false);
		select_position_button->SetValue(false);
		house_brush_button->Enable(false);
		house_brush_button->SetValue(false);
		edit_house_button->Enable(false);
		remove_house_button->Enable(false);
		g_gui.SelectBrush();
	}
}
#endif

// ============================================================================
// House Edit Dialog

BEGIN_EVENT_TABLE(EditHouseDialog, wxDialog)
EVT_SET_FOCUS(EditHouseDialog::OnFocusChange)
EVT_BUTTON(wxID_OK, EditHouseDialog::OnClickOK)
EVT_BUTTON(wxID_CANCEL, EditHouseDialog::OnClickCancel)
END_EVENT_TABLE()

EditHouseDialog::EditHouseDialog(wxWindow* parent, Map* map, House* house) :
	// window title
	wxDialog(parent, wxID_ANY, "House Properties", wxDefaultPosition, wxSize(250, 160)),
	map(map),
	what_house(house) {
	ASSERT(map);
	ASSERT(house);

	// main properties window box
	wxSizer* topsizer = newd wxBoxSizer(wxVERTICAL);
	wxSizer* boxsizer = newd wxStaticBoxSizer(wxVERTICAL, this, "House Properties");
	wxFlexGridSizer* housePropContainer = newd wxFlexGridSizer(2, 10, 10);
	housePropContainer->AddGrowableCol(1);

	wxFlexGridSizer* subsizer = newd wxFlexGridSizer(2, 10, 10);
	subsizer->AddGrowableCol(1);

	house_name = wxstr(house->name);
	house_id = i2ws(house->getID());
	house_rent = i2ws(house->rent);

	// House name
	subsizer->Add(newd wxStaticText(this, wxID_ANY, "Name:"), wxSizerFlags(0).Border(wxLEFT, 5));
	name_field = newd wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(160, 20), 0, wxTextValidator(wxFILTER_ASCII, &house_name));
	subsizer->Add(name_field, wxSizerFlags(1).Expand());

	// Town selection menu
	subsizer->Add(newd wxStaticText(this, wxID_ANY, "Town:"), wxSizerFlags(0).Border(wxLEFT, 5));

	const Towns& towns = map->towns;

	town_id_field = newd wxChoice(this, wxID_ANY);
	int to_select_index = 0;
	uint32_t houseTownId = house->townid;

	if (towns.count() > 0) {
		bool found = false;
		for (TownMap::const_iterator town_iter = towns.begin(); town_iter != towns.end(); ++town_iter) {
			if (town_iter->second->getID() == houseTownId) {
				found = true;
			}
			town_id_field->Append(wxstr(town_iter->second->getName()), newd int(town_iter->second->getID()));
			if (!found) {
				++to_select_index;
			}
		}

		if (!found) {
			if (houseTownId != 0) {
				town_id_field->Append("Undefined Town (id:" + i2ws(houseTownId) + ")", newd int(houseTownId));
				++to_select_index;
			}
		}
	}
	town_id_field->SetSelection(to_select_index);
	subsizer->Add(town_id_field, wxSizerFlags(1).Expand());
	// end town selection

	// Rent price
	subsizer->Add(newd wxStaticText(this, wxID_ANY, "Rent:"), wxSizerFlags(0).Border(wxLEFT, 5));
	rent_field = newd wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(160, 20), 0, wxTextValidator(wxFILTER_NUMERIC, &house_rent));
	subsizer->Add(rent_field, wxSizerFlags(1).Expand());

	// Right column
	wxFlexGridSizer* subsizerRight = newd wxFlexGridSizer(1, 10, 10);

	// house ID
	wxFlexGridSizer* houseSizer = newd wxFlexGridSizer(2, 10, 10);

	houseSizer->Add(newd wxStaticText(this, wxID_ANY, "ID:"), wxSizerFlags(0).Center());
	id_field = newd wxSpinCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(40, 20), wxSP_ARROW_KEYS, 1, 0xFFFF, house->getID());
	// id_field->Enable(false);
	houseSizer->Add(id_field, wxSizerFlags(1).Expand());
	subsizerRight->Add(houseSizer, wxSizerFlags(1).Expand());

	// Guildhall checkbox
	wxSizer* checkbox_sub_sizer = newd wxBoxSizer(wxVERTICAL);
	checkbox_sub_sizer->AddSpacer(4);

	guildhall_field = newd wxCheckBox(this, wxID_ANY, "Guildhall");

	checkbox_sub_sizer->Add(guildhall_field);
	subsizerRight->Add(checkbox_sub_sizer);
	guildhall_field->SetValue(house->guildhall);

	// construct the layout
	housePropContainer->Add(subsizer, wxSizerFlags(5).Expand());
	housePropContainer->Add(subsizerRight, wxSizerFlags(5).Expand());
	boxsizer->Add(housePropContainer, wxSizerFlags(5).Expand().Border(wxTOP | wxBOTTOM, 10));
	topsizer->Add(boxsizer, wxSizerFlags(0).Expand().Border(wxRIGHT | wxLEFT, 20));

	// OK/Cancel buttons
	wxSizer* buttonsSizer = newd wxBoxSizer(wxHORIZONTAL);
	buttonsSizer->Add(newd wxButton(this, wxID_OK, "OK"), wxSizerFlags(1).Center().Border(wxTOP | wxBOTTOM, 10));
	buttonsSizer->Add(newd wxButton(this, wxID_CANCEL, "Cancel"), wxSizerFlags(1).Center().Border(wxTOP | wxBOTTOM, 10));
	topsizer->Add(buttonsSizer, wxSizerFlags(0).Center().Border(wxLEFT | wxRIGHT, 20));

	SetSizerAndFit(topsizer);
}

EditHouseDialog::~EditHouseDialog() {
	////
}

void EditHouseDialog::OnFocusChange(wxFocusEvent& event) {
	wxWindow* win = event.GetWindow();
	if (wxSpinCtrl* spin = dynamic_cast<wxSpinCtrl*>(win)) {
		spin->SetSelection(-1, -1);
	} else if (wxTextCtrl* text = dynamic_cast<wxTextCtrl*>(win)) {
		text->SetSelection(-1, -1);
	}
}

void EditHouseDialog::OnClickOK(wxCommandEvent& WXUNUSED(event)) {
	if (Validate() && TransferDataFromWindow()) {
		// Verify the new rent information
		long new_house_rent;
		house_rent.ToLong(&new_house_rent);
		if (new_house_rent < 0) {
			g_gui.PopupDialog(this, "Error", "House rent cannot be less than 0.", wxOK);
			return;
		}

		// Verify the new house id
		uint32_t new_house_id = id_field->GetValue();
		if (new_house_id < 1) {
			g_gui.PopupDialog(this, "Error", "House id cannot be less than 1.", wxOK);
			return;
		}

		// Verify the new house name
		if (house_name.length() == 0) {
			g_gui.PopupDialog(this, "Error", "House name cannot be empty.", wxOK);
			return;
		}

		// Verify town selection
		if (town_id_field->GetSelection() == wxNOT_FOUND) {
			g_gui.PopupDialog(this, "Error", "You must select a town for this house.", wxOK);
			return;
		}

		int* new_town_id = reinterpret_cast<int*>(town_id_field->GetClientData(town_id_field->GetSelection()));
		if (!new_town_id) {
			g_gui.PopupDialog(this, "Error", "Invalid town selection.", wxOK);
			return;
		}

		if (g_settings.getInteger(Config::WARN_FOR_DUPLICATE_ID)) {
			Houses& houses = map->houses;
			for (HouseMap::const_iterator house_iter = houses.begin(); house_iter != houses.end(); ++house_iter) {
				House* house = house_iter->second;
				ASSERT(house);

				if (house->getID() == new_house_id && new_house_id != what_house->getID()) {
					g_gui.PopupDialog(this, "Error", "This house id is already in use.", wxOK);
					return;
				}

				if (wxstr(house->name) == house_name && house->getID() != what_house->getID()) {
					int ret = g_gui.PopupDialog(this, "Warning", "This house name is already in use, are you sure you want to continue?", wxYES | wxNO);
					if (ret == wxID_NO) {
						return;
					}
				}
			}
		}

		if (new_house_id != what_house->getID()) {
			int ret = g_gui.PopupDialog(this, "Warning", "Changing existing house ids on a production server WILL HAVE DATABASE CONSEQUENCES such as potential item loss, house owner change or invalidating guest lists.\nYou are doing it at own risk!\n\nAre you ABSOLUTELY sure you want to continue?", wxYES | wxNO);
			if (ret == wxID_NO) {
				return;
			}

			uint32_t old_house_id = what_house->getID();

			map->convertHouseTiles(old_house_id, new_house_id);
			map->houses.changeId(what_house, new_house_id);
		}

		// Transfer to house
		what_house->name = nstr(house_name);
		what_house->rent = new_house_rent;
		what_house->guildhall = guildhall_field->GetValue();
		what_house->townid = *new_town_id;

		EndModal(1);
	}
}

void EditHouseDialog::OnClickCancel(wxCommandEvent& WXUNUSED(event)) {
	// Just close this window
	EndModal(0);
}
