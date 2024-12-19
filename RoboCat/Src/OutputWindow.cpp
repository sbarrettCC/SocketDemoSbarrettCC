#include <RoboCatPCH.h>
#include <iostream>

OutputWindow::OutputWindow() : lineN(0)
{
    output = GetStdHandle(STD_OUTPUT_HANDLE);
   // MoveCursorToScreenBottom();
}

void OutputWindow::Write(std::string msg)
{
    _CONSOLE_SCREEN_BUFFER_INFO conInfo;
    GetConsoleScreenBufferInfo(output, &conInfo);
    SetConsoleCursorPosition(output, COORD{ 0, lineN });
    std::cout << msg;
    lineN++;
    if (lineN % (conInfo.dwSize.Y - 2) == 0)
    {
        short linesToScroll = std::min(short(5), short(conInfo.dwSize.Y - 2));
        Scroll(linesToScroll);
        lineN -= linesToScroll;
    }
    // ScrollConsoleScreenBuffer(output, )
    SetConsoleCursorPosition(output, conInfo.dwCursorPosition);
}

void OutputWindow::WriteFromStdin(std::string msg)
{
    MoveBackwards(); // to make up for \n in getline()
    ClearLine();
    Write(msg);
}

void OutputWindow::MoveBackwards(int lines)
{
    lineN -= short(lines);
}

void OutputWindow::ClearLine(int y)
{
    _CONSOLE_SCREEN_BUFFER_INFO conInfo;
    GetConsoleScreenBufferInfo(output, &conInfo);
    SetConsoleCursorPosition(output, COORD{ 0, short(conInfo.dwSize.Y - y) });
    for (int i = 0; i < conInfo.dwSize.X - 1; i++)
    {
        std::cout << " ";
    }
    SetConsoleCursorPosition(output, conInfo.dwCursorPosition);
}

int OutputWindow::Scroll(int lines)
{
    CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
    SMALL_RECT srctScrollRect, srctClipRect;
    CHAR_INFO chiFill;
    COORD coordDest;

    if (!GetConsoleScreenBufferInfo(output, &csbiInfo))
    {
        printf("GetConsoleScreenBufferInfo failed %d\n", GetLastError());
        return 1;
    }

    srctScrollRect.Top = short(lines); //csbiInfo.dwSize.Y - short(lines);
    srctScrollRect.Bottom = csbiInfo.dwSize.Y - 2;
    srctScrollRect.Left = 0;
    srctScrollRect.Right = csbiInfo.dwSize.X - 1;

    // The destination for the scroll rectangle is one row up.

    coordDest.X = 0;
    coordDest.Y = 0;

    // The clipping rectangle is the same as the scrolling rectangle.
    // The destination row is left unchanged.

    //srctClipRect = srctScrollRect;
    srctClipRect.Bottom = srctScrollRect.Bottom;
    srctClipRect.Top = 0;
    srctClipRect.Left = 0;
    srctClipRect.Right = srctScrollRect.Right;

    // Fill the bottom row with green blanks.

    chiFill.Attributes = 0; // BACKGROUND_GREEN | FOREGROUND_RED;
    chiFill.Char.AsciiChar = (char)' ';

    // Scroll up one line.

    if (!ScrollConsoleScreenBuffer(
        output,          // screen buffer handle
        &srctScrollRect, // scrolling rectangle
        &srctClipRect,   // clipping rectangle
        coordDest,       // top left destination cell
        &chiFill))       // fill character and color
    {
        printf("ScrollConsoleScreenBuffer failed %d\n", GetLastError());
        return 1;
    }

    return 0;
}

void OutputWindow::MoveCursorToScreenBottom()
{
    _CONSOLE_SCREEN_BUFFER_INFO conInfo;
    GetConsoleScreenBufferInfo(output, &conInfo);
    SetConsoleCursorPosition(output, COORD{ 0, short(conInfo.dwSize.Y - short(1)) });
}