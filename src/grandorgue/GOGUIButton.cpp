/*
 * GrandOrgue - free pipe organ simulator
 *
 * Copyright 2006 Milan Digital Audio LLC
 * Copyright 2009-2013 GrandOrgue contributors (see AUTHORS)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "GOGUIButton.h"

#include "GOGUIDisplayMetrics.h"
#include "GOGUIMouseState.h"
#include "GOGUIPanel.h"
#include "GOrgueButton.h"
#include "GOrgueConfigReader.h"
#include "GOrgueDC.h"
#include "GOrgueDocument.h"
#include "GrandOrgueFile.h"
#include <wx/intl.h>

GOGUIButton::GOGUIButton(GOGUIPanel* panel, GOrgueButton* control, bool is_piston, unsigned x_pos, unsigned y_pos) :
	GOGUIControl(panel, control),
	m_IsPiston(is_piston),
	m_Button(control),
	m_MouseRect(),
	m_Radius(0),
	m_FontSize(0),
	m_FontName(),
	m_TextColor(0,0,0),
	m_Text(),
	m_TextRect(),
	m_TextWidth(0),
	m_DispCol(x_pos),
	m_DispRow(y_pos),
	m_TileOffsetX(0),
	m_TileOffsetY(0)
{
}

void GOGUIButton::Init(GOrgueConfigReader& cfg, wxString group)
{
	GOGUIControl::Init(cfg, group);

	m_TextColor = wxColour(0x80, 0x00, 0x00);
	m_FontSize = 7;
	m_FontName = wxT("");
	m_Text = m_Button->GetName();

	int x, y, w, h;

	bool DispKeyLabelOnLeft = true;

	wxString off_mask_file, on_mask_file;
	wxString on_file, off_file;
	if (m_IsPiston)
	{
		int DispImageNum = m_Button->IsReadOnly() ? 3 : 1;
		off_file = wxString::Format(wxT("GO:piston%02d_off"), DispImageNum);
		on_file = wxString::Format(wxT("GO:piston%02d_on"), DispImageNum);
		
		m_metrics->GetPushbuttonBlitPosition(m_DispRow, m_DispCol, &x, &y);
		if (!DispKeyLabelOnLeft)
			x -= 13;
	}
	else
	{
		int DispImageNum = m_Button->IsReadOnly() ? 4 : 1;
		off_file = wxString::Format(wxT("GO:drawstop%02d_off"), DispImageNum);
		on_file = wxString::Format(wxT("GO:drawstop%02d_on"), DispImageNum);

		m_metrics->GetDrawstopBlitPosition(m_DispRow, m_DispCol, &x, &y);
	}

	on_mask_file = wxEmptyString;
	off_mask_file = on_mask_file;

	m_OnBitmap = m_panel->LoadBitmap(on_file, on_mask_file);
	m_OffBitmap = m_panel->LoadBitmap(off_file, off_mask_file);

	w = m_OnBitmap.GetWidth();
	h = m_OnBitmap.GetHeight();
	m_BoundingRect = wxRect(x, y, w, h);

	if (m_OnBitmap.GetWidth() != m_OffBitmap.GetWidth() ||
	    m_OnBitmap.GetHeight() != m_OffBitmap.GetHeight())
		throw wxString::Format(_("bitmap size does not match for '%s'"), group.c_str());

	m_TileOffsetX = 0;
	m_TileOffsetY = 0;

	x = 0;
	y = 0;
	w = m_BoundingRect.GetWidth() - x;
	h = m_BoundingRect.GetHeight() - y;
	m_MouseRect = wxRect(x + m_BoundingRect.GetX(), y + m_BoundingRect.GetY(), w, h);
	m_Radius = std::min(w/2, h/2);

	x = 1;
	y = 1;
	w = m_BoundingRect.GetWidth() - x;
	h = m_BoundingRect.GetHeight() - y;
	m_TextRect = wxRect(x + m_BoundingRect.GetX(), y + m_BoundingRect.GetY(), w, h);
	m_TextWidth = m_TextRect.GetWidth() - (m_TextRect.GetWidth() < 50 ? 4 : 14);

	m_Font = m_metrics->GetControlLabelFont();
	m_Font.SetName(m_FontName);
	m_Font.SetPoints(m_FontSize);
}

void GOGUIButton::Load(GOrgueConfigReader& cfg, wxString group)
{
	GOGUIControl::Load(cfg, group);
	m_IsPiston = cfg.ReadBoolean(ODFSetting, group, wxT("DisplayAsPiston"), false, m_IsPiston);

	m_TextColor = cfg.ReadColor(ODFSetting, group, wxT("DispLabelColour"), false, wxT("Dark Red"));
	m_FontSize = cfg.ReadFontSize(ODFSetting, group, wxT("DispLabelFontSize"), false, wxT("normal"));
	m_FontName = cfg.ReadString(ODFSetting, group, wxT("DispLabelFontName"), 255, false, wxT(""));
	m_Text = cfg.ReadString(ODFSetting, group, wxT("DispLabelText"), 255, false, m_Button->GetName());

	int x, y, w, h;

	bool DispKeyLabelOnLeft = cfg.ReadBoolean(ODFSetting, group, wxT("DispKeyLabelOnLeft"), false, true);

	wxString off_mask_file, on_mask_file;
	wxString on_file, off_file;
	if (m_IsPiston)
	{
		int DispImageNum = cfg.ReadInteger(ODFSetting, group, wxT("DispImageNum"), 1, 5, false, m_Button->IsReadOnly() ? 3 : 1);
		off_file = wxString::Format(wxT("GO:piston%02d_off"), DispImageNum);
		on_file = wxString::Format(wxT("GO:piston%02d_on"), DispImageNum);
		
		m_DispRow = cfg.ReadInteger(ODFSetting, group, wxT("DispButtonRow"), 0, 99 + m_metrics->NumberOfExtraButtonRows(), false, m_DispRow);
		m_DispCol = cfg.ReadInteger(ODFSetting, group, wxT("DispButtonCol"), 1, m_metrics->NumberOfButtonCols(), false, m_DispCol);
		m_metrics->GetPushbuttonBlitPosition(m_DispRow, m_DispCol, &x, &y);
		if (!DispKeyLabelOnLeft)
			x -= 13;
	}
	else
	{
		int DispImageNum = cfg.ReadInteger(ODFSetting, group, wxT("DispImageNum"), 1, 6, false, m_Button->IsReadOnly() ? 4 : 1);
		off_file = wxString::Format(wxT("GO:drawstop%02d_off"), DispImageNum);
		on_file = wxString::Format(wxT("GO:drawstop%02d_on"), DispImageNum);

		m_DispRow = cfg.ReadInteger(ODFSetting, group, wxT("DispDrawstopRow"), 1, 99 + m_metrics->NumberOfExtraDrawstopRowsToDisplay(), false, m_DispRow);
		m_DispCol = cfg.ReadInteger(ODFSetting, group, wxT("DispDrawstopCol"), 1, m_DispRow > 99 ? m_metrics->NumberOfExtraDrawstopColsToDisplay() : m_metrics->NumberOfDrawstopColsToDisplay(), false, m_DispCol);
		m_metrics->GetDrawstopBlitPosition(m_DispRow, m_DispCol, &x, &y);
	}

	on_file = cfg.ReadString(ODFSetting, group, wxT("ImageOn"), 255, false, on_file);
	off_file = cfg.ReadString(ODFSetting, group, wxT("ImageOff"), 255, false, off_file);
	on_mask_file = cfg.ReadString(ODFSetting, group, wxT("MaskOn"), 255, false, wxEmptyString);
	off_mask_file = cfg.ReadString(ODFSetting, group, wxT("MaskOff"), 255, false, on_mask_file);

	m_OnBitmap = m_panel->LoadBitmap(on_file, on_mask_file);
	m_OffBitmap = m_panel->LoadBitmap(off_file, off_mask_file);

	x = cfg.ReadInteger(ODFSetting, group, wxT("PositionX"), 0, m_metrics->GetScreenWidth(), false, x);
	y = cfg.ReadInteger(ODFSetting, group, wxT("PositionY"), 0, m_metrics->GetScreenHeight(), false, y);
	w = cfg.ReadInteger(ODFSetting, group, wxT("Width"), 1, m_metrics->GetScreenWidth(), false, m_OnBitmap.GetWidth());
	h = cfg.ReadInteger(ODFSetting, group, wxT("Height"), 1, m_metrics->GetScreenHeight(), false, m_OnBitmap.GetHeight());
	m_BoundingRect = wxRect(x, y, w, h);

	if (m_OnBitmap.GetWidth() != m_OffBitmap.GetWidth() ||
	    m_OnBitmap.GetHeight() != m_OffBitmap.GetHeight())
		throw wxString::Format(_("bitmap size does not match for '%s'"), group.c_str());

	m_TileOffsetX = cfg.ReadInteger(ODFSetting, group, wxT("TileOffsetX"), 0, m_OnBitmap.GetWidth() - 1, false, 0);
	m_TileOffsetY = cfg.ReadInteger(ODFSetting, group, wxT("TileOffsetY"), 0, m_OnBitmap.GetHeight() - 1, false, 0);

	x = cfg.ReadInteger(ODFSetting, group, wxT("MouseRectLeft"), 0, m_BoundingRect.GetWidth() - 1, false, 0);
	y = cfg.ReadInteger(ODFSetting, group, wxT("MouseRectTop"), 0, m_BoundingRect.GetHeight() - 1, false, 0);
	w = cfg.ReadInteger(ODFSetting, group, wxT("MouseRectWidth"), 1, m_BoundingRect.GetWidth() - x, false, m_BoundingRect.GetWidth() - x);
	h = cfg.ReadInteger(ODFSetting, group, wxT("MouseRectHeight"), 1, m_BoundingRect.GetHeight() - y, false, m_BoundingRect.GetHeight() - y);
	m_MouseRect = wxRect(x + m_BoundingRect.GetX(), y + m_BoundingRect.GetY(), w, h);
	m_Radius = cfg.ReadInteger(ODFSetting, group, wxT("MouseRadius"), 0, std::max(m_MouseRect.GetWidth(), m_MouseRect.GetHeight()), false, std::min(w/2, h/2));

	x = cfg.ReadInteger(ODFSetting, group, wxT("TextRectLeft"), 0, m_BoundingRect.GetWidth() - 1, false, 1);
	y = cfg.ReadInteger(ODFSetting, group, wxT("TextRectTop"), 0, m_BoundingRect.GetHeight() - 1, false, 1);
	w = cfg.ReadInteger(ODFSetting, group, wxT("TextRectWidth"), 1, m_BoundingRect.GetWidth() - x, false, m_BoundingRect.GetWidth() - x);
	h = cfg.ReadInteger(ODFSetting, group, wxT("TextRectHeight"), 1, m_BoundingRect.GetHeight() - y, false, m_BoundingRect.GetHeight() - y);
	m_TextRect = wxRect(x + m_BoundingRect.GetX(), y + m_BoundingRect.GetY(), w, h);
	m_TextWidth = cfg.ReadInteger(ODFSetting, group, wxT("TextBreakWidth"), 0, m_TextRect.GetWidth(), false, m_TextRect.GetWidth() - (m_TextRect.GetWidth() < 50 ? 4 : 14));

	m_Font = m_metrics->GetControlLabelFont();
	m_Font.SetName(m_FontName);
	m_Font.SetPoints(m_FontSize);
}

bool GOGUIButton::HandleMousePress(int x, int y, bool right, GOGUIMouseState& state)
{
	if (!m_MouseRect.Contains(x, y))
		return false;
	if (m_Radius)
	{
		if ((m_MouseRect.GetX() + m_Radius - x) * (m_MouseRect.GetX() + m_Radius - x) + (m_MouseRect.GetY() + m_Radius - y) * (m_MouseRect.GetY() + m_Radius - y) > m_Radius * m_Radius)
			return false;
	}
	if (right)
	{
		GOrgueMidiReceiver* midi = &m_Button->GetMidiReceiver();
		GOrgueMidiSender* sender = &m_Button->GetMidiSender();
		GOrgueKeyReceiver* key = &m_Button->GetKeyReceiver();
		wxString title;
		switch(midi->GetType())
		{
		case MIDI_RECV_DRAWSTOP:
			title = _("Midi-Settings for Drawstop - ") + m_Button->GetName();
			break;

		case MIDI_RECV_BUTTON:
			title = _("Midi-Settings for Pushbutton - ") + m_Button->GetName();
			break;

		default:
			title = _("Midi-Settings for Button - ") + m_Button->GetName();
		}

		if (m_Button->IsReadOnly())
		{
			midi = NULL;
			key = NULL;
		}

		GOrgueDocument* doc = m_panel->GetOrganFile()->GetDocument();
		doc->ShowMIDIEventDialog(&m_Button, title, midi, sender, key);
		return true;
	}
	else
	{
		if (state.GetControl() == this)
			return true;
		state.SetControl(this);
		if (m_Button->IsReadOnly())
			return true;

		m_Button->Push();
		return true;
	}
}

void GOGUIButton::Draw(GOrgueDC& dc)
{
	GOrgueBitmap& bmp = m_Button->DisplayInverted() ^ m_Button->IsEngaged() ? m_OnBitmap : m_OffBitmap;
	dc.TileBitmap(bmp, m_BoundingRect, m_TileOffsetX, m_TileOffsetY);
	if (m_TextWidth)
		dc.DrawText(m_Text, m_TextRect, m_TextColor, m_Font, m_TextWidth);

	GOGUIControl::Draw(dc);
}
