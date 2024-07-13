
#ifdef HUFFMANCODEC_GUI
// ./gui.ui_dx12.cpp
extern int init_gui(int, char**);
#else 
// ./cli/cmd.cpp
extern int init_cli(int, char**);
#endif

int main(int argc, char** argv) {
#ifdef HUFFMANCODEC_GUI
    init_gui(argc, argv);
#else
    init_cli(argc, argv);
#endif
}
