namespace console
{
    internal void cprint(const char* message);
    internal void cprintf(const char* fmt, ...);
    internal void command(char* text_command);
    internal bool is_shown();
    internal bool is_hidden();
}