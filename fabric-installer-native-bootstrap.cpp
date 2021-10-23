#include "Boostrap.h"

// Entrypoint
int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
    Installer::Boostrap bootstrap;
    bootstrap.launch();

    return 0;
}