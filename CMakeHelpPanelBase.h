//////////////////////////////////////////////////////////////////////
// This file was auto-generated by codelite's wxCrafter Plugin
// Do not modify this file by hand!
//////////////////////////////////////////////////////////////////////

#ifndef CMAKEHELPPANEL_BASE_CLASSES_H
#define CMAKEHELPPANEL_BASE_CLASSES_H

#include <wx/settings.h>
#include <wx/xrc/xmlres.h>
#include <wx/xrc/xh_bmp.h>
#include <wx/panel.h>
#include <wx/artprov.h>
#include <wx/sizer.h>
#include <wx/splitter.h>
#include <wx/listbox.h>
#include <wx/textctrl.h>

class CMakeHelpPanelBase : public wxPanel
{
protected:
    wxSplitterWindow* m_splitter;
    wxPanel* m_splitterPageList;
    wxListBox* m_listBoxList;
    wxPanel* m_splitterPageText;
    wxTextCtrl* m_textCtrlText;

protected:
    virtual void OnSelect(wxCommandEvent& event) { event.Skip(); }

public:
    CMakeHelpPanelBase(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(500,300), long style = wxTAB_TRAVERSAL);
    virtual ~CMakeHelpPanelBase();
};

#endif