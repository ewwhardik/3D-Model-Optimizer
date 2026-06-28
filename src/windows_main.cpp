#ifdef _WIN32
#include <windows.h>

// Forward declare real main
int main(int argc, char** argv);

int WINAPI WinMain(
    HINSTANCE /*hInstance*/,
    HINSTANCE /*hPrevInstance*/,
    LPSTR     /*lpCmdLine*/,
    int       /*nCmdShow*/)
{
    return main(__argc, __argv);
}
#endif
