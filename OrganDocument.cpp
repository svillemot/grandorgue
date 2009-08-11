/*
 * GrandOrgue - Copyright (C) 2009 JLDER - free pipe organ simulator based on MyOrgan Copyright (C) 2006 Kloria Publishing LLC
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "MyOrgan.h"

IMPLEMENT_DYNAMIC_CLASS(OrganDocument, wxDocument)

extern MySound* g_sound;
extern MyOrganFile* organfile;

OrganDocument::~OrganDocument(void)
{
	CloseOrgan();
	::wxGetApp().frame->SetTitle(APP_NAME);
}

bool OrganDocument::OnCloseDocument()
{
    if (!b_loaded)
        return false;
    return wxDocument::OnCloseDocument();
}

bool OrganDocument::DoOpenDocument(const wxString& file)
{
	return DoOpenDocument(file, wxEmptyString);
}

bool OrganDocument::DoOpenDocument(const wxString& file, const wxString& file2)
{
	wxBusyCursor busy;

	CloseOrgan();
	bool open_sound = g_sound->OpenSound(false);

	organfile = new MyOrganFile;
	wxString error = organfile->Load(file, file2);
	if (!error.IsEmpty())
	{
		CloseOrgan();
		if (!file2.IsEmpty())
            DeleteAllViews();
		if (error != "!")
			::wxLogError(error);
        b_loaded = true;
		return false;
	}

	SetTitle(organfile->ChurchName);

	Modify(false);
	UpdateAllViews(0, this);

	g_sound->b_active = open_sound;
	b_loaded = true;

	return true;
}

bool OrganDocument::DoSaveDocument(const wxString& file)
{
	organfile->Save(file);
	return true;
}

void OrganDocument::CloseOrgan()
{
	g_sound->CloseSound();
	if (organfile)
	{
		delete organfile;
		organfile = 0;
	}
	::wxGetApp().frame->m_meters[0]->m_meters[0]->ResetClip();
	::wxGetApp().frame->m_meters[0]->m_meters[1]->ResetClip();
	::wxGetApp().frame->m_meters[1]->m_meters[0]->ResetClip();
}
