#include "components/UI/Application.hpp"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

int main(void)
{
    Application app(SCREEN_WIDTH, SCREEN_HEIGHT);
    return app.run();
}
