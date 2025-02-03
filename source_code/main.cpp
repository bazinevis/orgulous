// To ensure fast compilation times, you may want to replace the below
// "include-all" headers with specific files.
#include "includes/RmlUi/Core.h"
#include "includes/RmlUi/Debugger.h"
#include <iostream>

class MyRenderInterface : public Rml::RenderInterface {
  /* ... */
};

class MySystemInterface : public Rml::SystemInterface {
  /* ... */
};


int main(int argc, char** argv)
{
  std::cout << "salam" <<std::endl;
  MySystemInterface *salam;
  
  // Initialize the window and graphics API being used, along with your game or
  // application.

  /* ... */

  // // Instantiate the interfaces to RmlUi.
  // MyRenderInterface render_interface;
  // MySystemInterface system_interface;

  // // Begin by installing the custom interfaces.
  // Rml::SetRenderInterface(&render_interface);
  // Rml::SetSystemInterface(&system_interface);

  // // Now we can initialize RmlUi.
  // Rml::Initialise();

  // // Create a context next.
  // Rml::Context *context =
  //     Rml::CreateContext("main", Rml::Vector2i(window_width, window_height));
  // if (!context) {
  //   Rml::Shutdown();
  //   return -1;
  // }

  // // If you want to use the debugger, initialize it now.
  // Rml::Debugger::Initialise(context);

  // // Fonts should be loaded before any documents are loaded.
  // Rml::LoadFontFace("my_font_file.otf");

  // // Now we are ready to load our document.
  // Rml::ElementDocument *document = context->LoadDocument("my_document.rml");
  // if (!document) {
  //   Rml::Shutdown();
  //   return -1;
  // }

  // document->Show();

  // bool exit_application = false;
  // while (!exit_application) {
  //   // We assume here that we have some way of updating and retrieving inputs
  //   // internally.
  //   my_input->Update();

  //   if (my_input->KeyPressed(KEY_ESC))
  //     exit_application = true;

  //   // Submit input events before the call to Context::Update().
  //   if (my_input->MouseMoved())
  //     context->ProcessMouseMove(mouse_pos.x, mouse_pos.y, 0);

  //   // Toggle the debugger with a key binding.
  //   if (my_input->KeyPressed(KEY_F8))
  //     Rml::Debugger::SetVisible(!Rml::Debugger::IsVisible());

  //   // This is a good place to update your game or application.
  //   my_application->Update();

  //   // Update any elements to reflect changed data.
  //   if (Rml::Element *el = document->GetElementById("score"))
  //     el->SetInnerRML("Current score: " + my_application->GetScoreAsString());

  //   // Update the context to reflect any changes resulting from input events,
  //   // animations, modified and added elements, or changed data in data
  //   // bindings.
  //   context->Update();

  //   // After the context update, the properties and layout of all elements are
  //   // properly resolved. At this point, we should no longer change any
  //   // elements, or submit input or other events until after the call to
  //   // Context::Render().

  //   // Render your game or application.
  //   my_application->Render();

  //   // Set up rendering states for RmlUi, use `Backend::BeginFrame` for the
  //   // built-in backends.
  //   my_renderer->BeginUserInterface();

  //   // Render the user interface on top of the application.
  //   context->Render();

  //   // Present the rendered frame, use `Backend::PresentFrame` for the built-in
  //   // backends.
  //   my_renderer->PresentFrame();
  // }

  // // Shutting down RmlUi releases all its resources, including elements,
  // // documents, and contexts.
  // Rml::Shutdown();

  // It is now safe to destroy the custom interfaces previously passed to RmlUi.

  return 0;
}
