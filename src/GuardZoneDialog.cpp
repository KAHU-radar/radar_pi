/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Navico BR24 Radar Plugin
 * Author:   David Register
 *           Dave Cowell
 *           Kees Verruijt
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register              bdbcat@yahoo.com *
 *   Copyright (C) 2012-2013 by Dave Cowell                                *
 *   Copyright (C) 2012-2013 by Kees Verruijt         canboat@verruijt.net *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************
 */

#include "br24radar_pi.h"

enum {                                      // process ID's
    ID_OK_Z,
    ID_ALARMZONES
};

bool    outer_set;

wxString GuardZoneNames[3];


//---------------------------------------------------------------------------------------
//          Guard Controls Implementation
//---------------------------------------------------------------------------------------

IMPLEMENT_CLASS(GuardZoneDialog, wxDialog)

BEGIN_EVENT_TABLE(GuardZoneDialog, wxDialog)

EVT_CLOSE(GuardZoneDialog::OnClose)
EVT_BUTTON(ID_OK_Z, GuardZoneDialog::OnIdOKClick)

END_EVENT_TABLE()

GuardZoneDialog::GuardZoneDialog()
{
    Init();
}

GuardZoneDialog::~GuardZoneDialog()
{
}

void GuardZoneDialog::Init()
{
}

bool GuardZoneDialog::Create(wxWindow *parent, br24radar_pi *pi, wxWindowID id,
                             const wxString  &m_caption, const wxPoint   &pos,
                             const wxSize    &size, long style)
{
    m_parent = parent;
    m_pi = pi;

#ifdef wxMSW
    long wstyle = wxSYSTEM_MENU | wxCLOSE_BOX | wxCAPTION | wxCLIP_CHILDREN;
#else
    long wstyle =                 wxCLOSE_BOX | wxCAPTION | wxCLIP_CHILDREN;
#endif

    wxSize  size_min = wxSize(200, 200);

    if (!wxDialog::Create(parent, id, m_caption, pos, size_min, wstyle)) return false;

    CreateControls();

    DimeWindow(this);

    Fit();
    SetMinSize(GetBestSize());

    return true;
}


void GuardZoneDialog::CreateControls()
{
    static const int border_size = 4;

    wxBoxSizer *GuardZoneSizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(GuardZoneSizer);

    // Guard Zone options
    pBoxGuardZone = new wxStaticBox(this, wxID_ANY, _("Guard zones"));
    wxStaticBoxSizer    *BoxGuardZoneSizer = new wxStaticBoxSizer(pBoxGuardZone, wxVERTICAL);
    GuardZoneSizer->Add(BoxGuardZoneSizer, 0, wxEXPAND | wxALL, border_size);

    GuardZoneNames[0] = _("Off");
    GuardZoneNames[1] = _("Arc");
    GuardZoneNames[2] = _("Circle");
    pGuardZoneType = new wxRadioBox (this, ID_ALARMZONES, _("Zone type:"),
                                     wxDefaultPosition, wxDefaultSize,
                                     ARRAY_SIZE(GuardZoneNames), GuardZoneNames, 1, wxRA_SPECIFY_COLS );

    BoxGuardZoneSizer->Add(pGuardZoneType, 0, wxALL | wxEXPAND, 2);

    pGuardZoneType->Connect( wxEVT_COMMAND_RADIOBOX_SELECTED,
                            wxCommandEventHandler(GuardZoneDialog::OnGuardZoneModeClick),
                            NULL, this );

    //Inner and Outer Ranges
    wxString m_temp;
    wxStaticText *pInner_Range_Text = new wxStaticText(this, wxID_ANY, _("Inner range"),wxDefaultPosition,
                                                       wxDefaultSize, 0);
    BoxGuardZoneSizer->Add(pInner_Range_Text, 0, wxALIGN_LEFT | wxALL, 0);

    pInner_Range = new wxTextCtrl(this, wxID_ANY);
    BoxGuardZoneSizer->Add(pInner_Range, 1, wxALIGN_LEFT | wxALL, 5);
    pInner_Range->Connect(wxEVT_COMMAND_TEXT_UPDATED,
                          wxCommandEventHandler(GuardZoneDialog::OnInner_Range_Value), NULL, this);
///   start of copy
    wxStaticText *pOuter_Range_Text = new wxStaticText(this, wxID_ANY, _("Outer range"),wxDefaultPosition,
                                                       wxDefaultSize, 0);
    BoxGuardZoneSizer->Add(pOuter_Range_Text, 0, wxALIGN_LEFT | wxALL, 0);

    pOuter_Range = new wxTextCtrl(this, wxID_ANY);
    BoxGuardZoneSizer->Add(pOuter_Range, 1, wxALIGN_LEFT | wxALL, 5);
    pOuter_Range->Connect(wxEVT_COMMAND_TEXT_UPDATED,
                          wxCommandEventHandler(GuardZoneDialog::OnOuter_Range_Value), NULL, this);

    //1st and 2nd Arc Subtending Bearings
    wxStaticText *pStart_Bearing = new wxStaticText(this, wxID_ANY, _("Start bearing"),wxDefaultPosition,
                                                    wxDefaultSize, 0);
    BoxGuardZoneSizer->Add(pStart_Bearing, 0, wxALIGN_LEFT | wxALL, 0);

    pStart_Bearing_Value = new wxTextCtrl(this, wxID_ANY);
    BoxGuardZoneSizer->Add(pStart_Bearing_Value, 1, wxALIGN_LEFT | wxALL, 5);
    pStart_Bearing_Value->Connect(wxEVT_COMMAND_TEXT_UPDATED,
                                  wxCommandEventHandler(GuardZoneDialog::OnStart_Bearing_Value), NULL, this);

/////////////////////////////////////////
    wxStaticText *pEnd_Bearing = new wxStaticText(this, wxID_ANY, _("End bearing"),wxDefaultPosition,
                                                  wxDefaultSize, 0);
    BoxGuardZoneSizer->Add(pEnd_Bearing, 0, wxALIGN_LEFT | wxALL, 0);

    pEnd_Bearing_Value = new wxTextCtrl(this, wxID_ANY);
    BoxGuardZoneSizer->Add(pEnd_Bearing_Value, 1, wxALIGN_LEFT | wxALL, 5);
    pEnd_Bearing_Value->Connect(wxEVT_COMMAND_TEXT_UPDATED,
                                wxCommandEventHandler(GuardZoneDialog::OnEnd_Bearing_Value), NULL, this);

     //  Options
 //   wxString label;
 //   label << _("Warning: Targets may be") << wxT("\n\r") << _("lost with filter on");
    wxStaticBox* itemStaticBoxOptions = new wxStaticBox(this, wxID_ANY, _("Filter"));
    wxStaticBoxSizer* itemStaticBoxSizerOptions = new wxStaticBoxSizer(itemStaticBoxOptions, wxVERTICAL);
    GuardZoneSizer->Add(itemStaticBoxSizerOptions, 0, wxEXPAND | wxALL, border_size);

    wxStaticText *pStatic_Warning = new wxStaticText(this, wxID_ANY, _("Warning: Targets may be\nmissed with filter on"));
    itemStaticBoxSizerOptions->Add(pStatic_Warning, 1, wxALIGN_LEFT | wxALL, 2);

    // added check box to control multi swep filtering
    cbFilter = new wxCheckBox(this, wxID_ANY, _("Multi Sweep Filter On"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE | wxST_NO_AUTORESIZE);
    itemStaticBoxSizerOptions->Add(cbFilter, 0, wxALIGN_CENTER_VERTICAL | wxALL, border_size);
//    int test = m_pi->settings.PassHeadingToOCPN;
//    cbFilter->SetValue(m_pi->settings.PassHeadingToOCPN ? true : false);
    cbFilter->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED,
                             wxCommandEventHandler(GuardZoneDialog::OnFilterClick), NULL, this);

    // The Close button
    wxButton    *bClose = new wxButton(this, ID_OK_Z, _("&Close"), wxDefaultPosition, wxDefaultSize, 0);
    GuardZoneSizer->Add(bClose, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

}

//*********************************************************************************************************************

void GuardZoneDialog::SetVisibility()
{
    GuardZoneType zoneType = (GuardZoneType) pGuardZoneType->GetSelection();

    m_guard_zone->type = zoneType;

    if (zoneType == GZ_OFF) {
        pStart_Bearing_Value->Enable();
        pEnd_Bearing_Value->Enable();
        pInner_Range->Enable();
        pOuter_Range->Enable();

    } else if (pGuardZoneType->GetSelection() == GZ_CIRCLE) {
        pStart_Bearing_Value->Disable();
        pEnd_Bearing_Value->Disable();
        pInner_Range->Enable();
        pOuter_Range->Enable();

    }
    else {
        pStart_Bearing_Value->Enable();
        pEnd_Bearing_Value->Enable();
        pInner_Range->Enable();
        pOuter_Range->Enable();

    }
}


void GuardZoneDialog::OnGuardZoneDialogShow(RadarInfo * ri, int zone)
{
    double conversionFactor = RangeUnitsToMeters[m_pi->m_settings.range_units];

    m_ri = ri;
    m_guard_zone = ri->guard_zone[zone];

    wxString GuardZoneText;
    wxString t;
    t.Printf(_T(" %d"), zone + 1);
    GuardZoneText << _("Guard Zone") << t;
    pBoxGuardZone->SetLabel(GuardZoneText);

    pGuardZoneType->SetSelection(m_guard_zone->type);

    GuardZoneText.Printf(wxT("%2.2f"), m_guard_zone->inner_range / conversionFactor);
    pInner_Range->SetValue(GuardZoneText);

    GuardZoneText.Printf(wxT("%2.2f"), m_guard_zone->outer_range / conversionFactor);
    pOuter_Range->SetValue(GuardZoneText);

    GuardZoneText.Printf(wxT("%3.1f"), SCALE_RAW_TO_DEGREES2048(m_guard_zone->start_bearing));
    pStart_Bearing_Value->SetValue(GuardZoneText);
    GuardZoneText.Printf(wxT("%3.1f"), SCALE_RAW_TO_DEGREES2048(m_guard_zone->end_bearing));
    pEnd_Bearing_Value->SetValue(GuardZoneText);

    cbFilter->SetValue(m_guard_zone->multi_sweep_filter ? 1 : 0);

    SetVisibility();
}

void GuardZoneDialog::OnGuardZoneModeClick(wxCommandEvent &event)
{
    SetVisibility();
    outer_set = false;
}

void GuardZoneDialog::OnInner_Range_Value(wxCommandEvent &event)
{
    wxString temp = pInner_Range->GetValue();
    double t;
    temp.ToDouble(&t);

    int conversionFactor = RangeUnitsToMeters[m_pi->m_settings.range_units];

    m_guard_zone->inner_range = (int)(t * conversionFactor);
}

void GuardZoneDialog::OnOuter_Range_Value(wxCommandEvent &event)
{
    wxString temp = pOuter_Range->GetValue();
    double t;
    temp.ToDouble(&t);

    int conversionFactor = RangeUnitsToMeters[m_pi->m_settings.range_units];

    m_guard_zone->outer_range = (int)(t * conversionFactor);
}

void GuardZoneDialog::OnStart_Bearing_Value(wxCommandEvent &event)
{
    wxString temp = pStart_Bearing_Value->GetValue();
    double t;

    temp.ToDouble(&t);
    m_guard_zone->start_bearing = SCALE_DEGREES_TO_RAW2048(t);
}


void GuardZoneDialog::OnEnd_Bearing_Value(wxCommandEvent &event)
{
    wxString temp = pEnd_Bearing_Value->GetValue();
    double t;

    temp.ToDouble(&t);
    m_guard_zone->end_bearing = SCALE_DEGREES_TO_RAW2048(t);
}

void GuardZoneDialog::OnFilterClick(wxCommandEvent &event)
{
    int filt = cbFilter->GetValue();
    m_guard_zone->multi_sweep_filter = filt;
}

void GuardZoneDialog::OnClose(wxCloseEvent &event)
{
    m_pi->OnGuardZoneDialogClose(m_ri);
    event.Skip();
}

void GuardZoneDialog::OnIdOKClick(wxCommandEvent &event)
{
    m_pi->OnGuardZoneDialogClose(m_ri);
    event.Skip();
}