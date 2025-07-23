
#include <Stapel/Stapel.h>
#include <Window/Window.h>
#include <Renderer/Renderer.h>

#include <memory>

extern stapel::IApplication* GetApplication();

extern "C" int stapel_engine_entry(int argc, char **argv)
{
    stapel::IApplication* app = GetApplication();
    app->PreEngineInitHook(argc, argv);

    stapel::RendererSpecification spec {
        .app = app->GetApplicationInfo(),
    };

    stapel::WindowSpecification winspec {};
    app->WindowCreateSpecHook(winspec);

    auto window = std::make_shared<stapel::Window>(winspec);
    stapel::Renderer renderer(window, spec);

    return 0;
}
