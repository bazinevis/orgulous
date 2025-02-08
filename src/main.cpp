#include "includes/RmlUi/Core/Context.h"
#include "includes/RmlUi/Core/Core.h"
#include "includes/RmlUi/Core/ElementDocument.h"
#include "includes/RmlUi/Core/FileInterface.h"
#include "includes/RmlUi/Core/RenderInterface.h"
#include "includes/RmlUi/Core/SystemInterface.h"
#include "includes/RmlUi/Core/Types.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>

using namespace Rml;

static void SetRenderClipRect(SDL_Renderer *renderer, const SDL_Rect *rect) {
#if SDL_MAJOR_VERSION >= 3
  SDL_SetRenderClipRect(renderer, rect);
#else
  SDL_RenderSetClipRect(renderer, rect);
#endif
}

static void SetRenderViewport(SDL_Renderer *renderer, const SDL_Rect *rect) {
#if SDL_MAJOR_VERSION >= 3
  SDL_SetRenderV iewport(renderer, rect);
#else
  SDL_RenderSetViewport(renderer, rect);
#endif
}

class OrgulousRenderInterface : public Rml::RenderInterface {
public:
  OrgulousRenderInterface(SDL_Renderer *renderer) { this->renderer = renderer; }

  Rml::CompiledGeometryHandle
  CompileGeometry(Rml::Span<const Rml::Vertex> vertices,
                  Rml::Span<const int> indices) {
    GeometryView *data = new GeometryView{vertices, indices};
    return reinterpret_cast<Rml::CompiledGeometryHandle>(data);
  }

  void RenderGeometry(CompiledGeometryHandle handle, Vector2f translation,
                      TextureHandle texture) {
    const GeometryView *geometry = reinterpret_cast<GeometryView *>(handle);
    const Rml::Vertex *vertices = geometry->vertices.data();
    const size_t num_vertices = geometry->vertices.size();
    const int *indices = geometry->indices.data();
    const size_t num_indices = geometry->indices.size();

    Rml::UniquePtr<SDL_Vertex[]> sdl_vertices{new SDL_Vertex[num_vertices]};

    for (size_t i = 0; i < num_vertices; i++) {
      sdl_vertices[i].position = {vertices[i].position.x + translation.x,
                                  vertices[i].position.y + translation.y};
      sdl_vertices[i].tex_coord = {vertices[i].tex_coord.x,
                                   vertices[i].tex_coord.y};

      const auto &color = vertices[i].colour;
      sdl_vertices[i].color = {color.red, color.green, color.blue, color.alpha};

      SDL_Texture *sdl_texture = (SDL_Texture *)texture;

      SDL_RenderGeometry(renderer, sdl_texture, sdl_vertices.get(),
                         (int)num_vertices, indices, (int)num_indices);
    }
  };

  void ReleaseGeometry(CompiledGeometryHandle geometry) {
    delete reinterpret_cast<GeometryView *>(geometry);
  };

  TextureHandle LoadTexture(Vector2i &texture_dimensions,
                            const String &source) {
    Rml::FileInterface *file_interface = Rml::GetFileInterface();
    Rml::FileHandle file_handle = file_interface->Open(source);
    if (!file_handle)
      return {};

    file_interface->Seek(file_handle, 0, SEEK_END);
    size_t buffer_size = file_interface->Tell(file_handle);
    file_interface->Seek(file_handle, 0, SEEK_SET);

    using Rml::byte;
    Rml::UniquePtr<byte[]> buffer(new byte[buffer_size]);
    file_interface->Read(buffer.get(), buffer_size, file_handle);
    file_interface->Close(file_handle);

    const size_t i_ext = source.rfind('.');
    Rml::String extension =
      (i_ext == Rml::String::npos ? Rml::String() : source.substr(i_ext + 1));

    auto CreateSurface = [&]() {
      return IMG_LoadTyped_RW(SDL_RWFromMem(buffer.get(), int(buffer_size)), 1,
                              extension.c_str());
    };
    auto GetSurfaceFormat = [](SDL_Surface *surface) {
      return surface->format->format;
    };
    auto ConvertSurface = [](SDL_Surface *surface, Uint32 format) {
      return SDL_ConvertSurfaceFormat(surface, format, 0);
    };
    auto DestroySurface = [](SDL_Surface *surface) {
      SDL_FreeSurface(surface);
    };

    SDL_Surface *surface = CreateSurface();
    if (!surface)
      return {};

    texture_dimensions = {surface->w, surface->h};

    if (GetSurfaceFormat(surface) != SDL_PIXELFORMAT_RGBA32 &&
        GetSurfaceFormat(surface) != SDL_PIXELFORMAT_BGRA32) {
      SDL_Surface *converted_surface =
        ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32);
      DestroySurface(surface);
      if (!converted_surface)
        return {};

      surface = converted_surface;
    }

    // Convert colors to premultiplied alpha, which is necessary for correct
    // alpha compositing.
    const size_t pixels_byte_size = surface->w * surface->h * 4;
    byte *pixels = static_cast<byte *>(surface->pixels);
    for (size_t i = 0; i < pixels_byte_size; i += 4) {
      const byte alpha = pixels[i + 3];
      for (size_t j = 0; j < 3; ++j)
        pixels[i + j] = byte(int(pixels[i + j]) * int(alpha) / 255);
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    texture_dimensions = Rml::Vector2i(surface->w, surface->h);
    DestroySurface(surface);

    if (texture)
      SDL_SetTextureBlendMode(texture, blend_mode);

    return (Rml::TextureHandle)texture;
  };

  TextureHandle GenerateTexture(Span<const byte> source,
                                Vector2i source_dimensions) {
    RMLUI_ASSERT(source.data() &&
                 source.size() ==
                 size_t(source_dimensions.x * source_dimensions.y * 4));

    auto CreateSurface = [&]() {
      return SDL_CreateRGBSurfaceWithFormatFrom(
                                                (void *)source.data(), source_dimensions.x, source_dimensions.y, 32,
                                                source_dimensions.x * 4, SDL_PIXELFORMAT_RGBA32);
    };
    auto DestroySurface = [](SDL_Surface *surface) {
      SDL_FreeSurface(surface);
    };

    SDL_Surface *surface = CreateSurface();

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_SetTextureBlendMode(texture, blend_mode);

    DestroySurface(surface);
    return (Rml::TextureHandle)texture;
  };

  void ReleaseTexture(TextureHandle texture) {
    SDL_DestroyTexture((SDL_Texture *)texture);
  };

  void EnableScissorRegion(bool enable) {
    if (enable)
      SetRenderClipRect(renderer, &rect_scissor);
    else
      SetRenderClipRect(renderer, nullptr);

    scissor_region_enabled = enable;
  };

  void SetScissorRegion(Rectanglei region) {
    rect_scissor.x = region.Left();
    rect_scissor.y = region.Top();
    rect_scissor.w = region.Width();
    rect_scissor.h = region.Height();

    if (scissor_region_enabled)
      SetRenderClipRect(renderer, &rect_scissor);
  };

private:
  struct GeometryView {
    Rml::Span<const Rml::Vertex> vertices;
    Rml::Span<const int> indices;
  };

  SDL_Renderer *renderer;
  SDL_BlendMode blend_mode = {};
  SDL_Rect rect_scissor = {};
  bool scissor_region_enabled = false;
};

class OrgulousSystemInterface : public Rml::SystemInterface {};

int main(int argc, char **argv)

{
  int window_width = 680;
  int window_height = 400;
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
  SDL_Window *window = SDL_CreateWindow("Orgulous", 0, 0, 680, 400, 0);
  SDL_Renderer *renderer =
    SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  // Instantiate the interfaces to RmlUi.
  OrgulousRenderInterface render_interface(renderer);
  OrgulousSystemInterface system_interface;

  Rml::Initialise();
  Rml::SetRenderInterface(&render_interface);
  Rml::SetSystemInterface(&system_interface);

  Rml::LoadFontFace("fonts/OpenSans.ttf");

  // Create a context next.
  Rml::Context *context =
    Rml::CreateContext("main", Rml::Vector2i(window_width, window_height));
  if (!context) {
    Rml::Shutdown();
    return -1;
  }

  // Now we are ready to load our document.
  Rml::ElementDocument *document = context->LoadDocument("rml/ui.rml");
  if (!document) {
    Rml::Shutdown();
    return -1;
  }

  document->Show();

  bool running = true;
  SDL_Event event;
  while (running) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        running = false;
      }
    }
    context->Update();

    SetRenderViewport(renderer, nullptr);
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 0);
    SDL_RenderClear(renderer);
    context->Render();
    SDL_RenderPresent(renderer);
  }

  Rml::Shutdown();
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
