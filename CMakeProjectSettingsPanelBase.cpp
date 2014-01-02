//////////////////////////////////////////////////////////////////////
// This file was auto-generated by codelite's wxCrafter Plugin
// Do not modify this file by hand!
//////////////////////////////////////////////////////////////////////

#include "CMakeProjectSettingsPanelBase.h"


// Declare the bitmap loading function
extern void wxCrafterg7ypzmInitBitmapResources();

static bool bBitmapLoaded = false;


CMakeProjectSettingsPanelBase::CMakeProjectSettingsPanelBase(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
    : wxPanel(parent, id, pos, size, style)
{
    if ( !bBitmapLoaded ) {
        // We need to initialise the default bitmap handler
        wxXmlResource::Get()->AddHandler(new wxBitmapXmlHandler);
        wxCrafterg7ypzmInitBitmapResources();
        bBitmapLoaded = true;
    }
    
    wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(boxSizer);
    
    m_checkBoxEnable = new wxCheckBox(this, wxID_ANY, _("Enable CMake for this project"), wxDefaultPosition, wxSize(-1,-1), 0);
    m_checkBoxEnable->SetValue(false);
    
    boxSizer->Add(m_checkBoxEnable, 0, wxALL, 5);
    
    wxFlexGridSizer* flexGridSizer = new wxFlexGridSizer(  5, 2, 5, 5);
    flexGridSizer->SetFlexibleDirection( wxBOTH );
    flexGridSizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );
    flexGridSizer->AddGrowableCol(1);
    
    boxSizer->Add(flexGridSizer, 0, wxALL|wxEXPAND, 5);
    
    m_staticTextParent = new wxStaticText(this, wxID_ANY, _("Parent project:"), wxDefaultPosition, wxSize(-1,-1), 0);
    
    flexGridSizer->Add(m_staticTextParent, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 0);
    
    wxArrayString m_choiceParentArr;
    m_choiceParent = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxSize(-1,-1), m_choiceParentArr, 0);
    
    flexGridSizer->Add(m_choiceParent, 0, wxALL|wxEXPAND, 0);
    
    m_staticTextSourceDir = new wxStaticText(this, wxID_ANY, _("Sources directory:"), wxDefaultPosition, wxSize(-1,-1), 0);
    
    flexGridSizer->Add(m_staticTextSourceDir, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 0);
    
    m_dirPickerSourceDir = new wxDirPickerCtrl(this, wxID_ANY, wxEmptyString, wxT("Select a folder"), wxDefaultPosition, wxSize(-1,-1), wxDIRP_DEFAULT_STYLE|wxDIRP_USE_TEXTCTRL);
    m_dirPickerSourceDir->SetToolTip(_("Directory must contains CMakeLists.txt"));
    
    flexGridSizer->Add(m_dirPickerSourceDir, 0, wxALL|wxEXPAND|wxALIGN_CENTER_VERTICAL, 0);
    
    m_staticTextBuildDir = new wxStaticText(this, wxID_ANY, _("Build directory:"), wxDefaultPosition, wxSize(-1,-1), 0);
    
    flexGridSizer->Add(m_staticTextBuildDir, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 0);
    
    m_dirPickerBuildDir = new wxDirPickerCtrl(this, wxID_ANY, wxEmptyString, wxT("Select a folder"), wxDefaultPosition, wxSize(-1,-1), wxDIRP_DEFAULT_STYLE|wxDIRP_USE_TEXTCTRL);
    
    flexGridSizer->Add(m_dirPickerBuildDir, 0, wxALL|wxEXPAND, 0);
    
    m_staticTextGenerator = new wxStaticText(this, wxID_ANY, _("Generator:"), wxDefaultPosition, wxSize(-1,-1), 0);
    
    flexGridSizer->Add(m_staticTextGenerator, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 0);
    
    wxArrayString m_choiceGeneratorArr;
    m_choiceGenerator = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxSize(-1,-1), m_choiceGeneratorArr, 0);
    
    flexGridSizer->Add(m_choiceGenerator, 0, wxALL|wxEXPAND, 0);
    
    m_staticTextBuildType = new wxStaticText(this, wxID_ANY, _("Build Type:"), wxDefaultPosition, wxSize(-1,-1), 0);
    
    flexGridSizer->Add(m_staticTextBuildType, 0, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 0);
    
    wxArrayString m_comboBoxBuildTypeArr;
    m_comboBoxBuildTypeArr.Add(wxT("None"));
    m_comboBoxBuildTypeArr.Add(wxT("Debug"));
    m_comboBoxBuildTypeArr.Add(wxT("Release"));
    m_comboBoxBuildTypeArr.Add(wxT("RelWithDebInfo"));
    m_comboBoxBuildTypeArr.Add(wxT("MinSizeRel"));
    m_comboBoxBuildType = new wxComboBox(this, wxID_ANY, wxT(""), wxDefaultPosition, wxSize(-1,-1), m_comboBoxBuildTypeArr, 0);
    
    flexGridSizer->Add(m_comboBoxBuildType, 0, wxALL|wxEXPAND, 0);
    
    m_staticTextArguments = new wxStaticText(this, wxID_ANY, _("CMake arguments (used for configuration)"), wxDefaultPosition, wxSize(-1,-1), 0);
    
    boxSizer->Add(m_staticTextArguments, 0, wxALL, 5);
    
    m_textCtrlArguments = new wxTextCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, wxSize(-1,-1), wxTE_MULTILINE);
    
    boxSizer->Add(m_textCtrlArguments, 1, wxALL|wxEXPAND, 5);
    
    SetMinSize( wxSize(400,300) );
    SetSizeHints(400,300);
    if ( GetSizer() ) {
         GetSizer()->Fit(this);
    }
    Centre(wxBOTH);
    // Connect events
    m_staticTextParent->Connect(wxEVT_UPDATE_UI, wxUpdateUIEventHandler(CMakeProjectSettingsPanelBase::OnCheck), NULL, this);
    m_choiceParent->Connect(wxEVT_UPDATE_UI, wxUpdateUIEventHandler(CMakeProjectSettingsPanelBase::OnCheck), NULL, this);
    m_staticTextSourceDir->Connect(wxEVT_UPDATE_UI, wxUpdateUIEventHandler(CMakeProjectSettingsPanelBase::OnCheck2), NULL, this);
    m_dirPickerSourceDir->Connect(wxEVT_UPDATE_UI, wxUpdateUIEventHandler(CMakeProjectSettingsPanelBase::OnCheck2), NULL, this);
    m_staticTextBuildDir->Connect(wxEVT_UPDATE_UI, wxUpdateUIEventHandler(CMakeProjectSettingsPanelBase::OnCheck2), NULL, this);
    m_dirPickerBuildDir->Connect(wxEVT_UPDATE_UI, wxUpdateUIEventHandler(CMakeProjectSettingsPanelBase::OnCheck2), NULL, this);
    m_staticTextGenerator->Connect(wxEVT_UPDATE_UI, wxUpdateUIEventHandler(CMakeProjectSettingsPanelBase::OnCheck2), NULL, this);
    m_staticTextBuildType->Connect(wxEVT_UPDATE_UI, wxUpdateUIEventHandler(CMakeProjectSettingsPanelBase::OnCheck2), NULL, this);
    m_comboBoxBuildType->Connect(wxEVT_UPDATE_UI, wxUpdateUIEventHandler(CMakeProjectSettingsPanelBase::OnCheck2), NULL, this);
    m_staticTextArguments->Connect(wxEVT_UPDATE_UI, wxUpdateUIEventHandler(CMakeProjectSettingsPanelBase::OnCheck2), NULL, this);
    m_textCtrlArguments->Connect(wxEVT_UPDATE_UI, wxUpdateUIEventHandler(CMakeProjectSettingsPanelBase::OnCheck2), NULL, this);
    
}

CMakeProjectSettingsPanelBase::~CMakeProjectSettingsPanelBase()
{
    m_staticTextParent->Disconnect(wxEVT_UPDATE_UI, wxUpdateUIEventHandler(CMakeProjectSettingsPanelBase::OnCheck), NULL, this);
    m_choiceParent->Disconnect(wxEVT_UPDATE_UI, wxUpdateUIEventHandler(CMakeProjectSettingsPanelBase::OnCheck), NULL, this);
    m_staticTextSourceDir->Disconnect(wxEVT_UPDATE_UI, wxUpdateUIEventHandler(CMakeProjectSettingsPanelBase::OnCheck2), NULL, this);
    m_dirPickerSourceDir->Disconnect(wxEVT_UPDATE_UI, wxUpdateUIEventHandler(CMakeProjectSettingsPanelBase::OnCheck2), NULL, this);
    m_staticTextBuildDir->Disconnect(wxEVT_UPDATE_UI, wxUpdateUIEventHandler(CMakeProjectSettingsPanelBase::OnCheck2), NULL, this);
    m_dirPickerBuildDir->Disconnect(wxEVT_UPDATE_UI, wxUpdateUIEventHandler(CMakeProjectSettingsPanelBase::OnCheck2), NULL, this);
    m_staticTextGenerator->Disconnect(wxEVT_UPDATE_UI, wxUpdateUIEventHandler(CMakeProjectSettingsPanelBase::OnCheck2), NULL, this);
    m_staticTextBuildType->Disconnect(wxEVT_UPDATE_UI, wxUpdateUIEventHandler(CMakeProjectSettingsPanelBase::OnCheck2), NULL, this);
    m_comboBoxBuildType->Disconnect(wxEVT_UPDATE_UI, wxUpdateUIEventHandler(CMakeProjectSettingsPanelBase::OnCheck2), NULL, this);
    m_staticTextArguments->Disconnect(wxEVT_UPDATE_UI, wxUpdateUIEventHandler(CMakeProjectSettingsPanelBase::OnCheck2), NULL, this);
    m_textCtrlArguments->Disconnect(wxEVT_UPDATE_UI, wxUpdateUIEventHandler(CMakeProjectSettingsPanelBase::OnCheck2), NULL, this);
    
}
