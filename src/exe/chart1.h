#ifndef __CHART1_H
#define __CHART1_H
#include "wx/wx.h"

class PlugInManager;
class wxGLCanvas;
class wxGLContext;

    // https://www.kirix.com/labs/wxaui/documentation/examples.html
class MyFrame : public wxFrame {
public:
  MyFrame(wxWindow* parent);
   ~MyFrame();

  wxAuiManager m_mgr;
  PlugInManager* m_PluginManager;
  wxTimer* RenderTimer;
  wxGLCanvas* m_glChartCanvas;
  wxGLContext* m_glContext;
  void OnRenderTimer(wxTimerEvent& event);
  void OnCloseWindow(wxCloseEvent& event);

  DECLARE_EVENT_TABLE();
};
#endif
