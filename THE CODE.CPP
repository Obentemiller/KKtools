#include <iostream>
#include <conio.h>
#include <windows.h>
#include <random>
#include <shlobj.h>
#include <map>
#include <vector>

#define RESET "\033[0m"
#define GREEN "\033[32m"  

bool S_DAF;

static void randowblimp(int ini, int fim, int probabilityOfStar) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(ini, fim);

    for (int i = ini; i <= fim; i++) {
        if (distrib(gen) <= probabilityOfStar) {
            std::cout << "*";
        }
        else {
            std::cout << " ";
        }
    }
    std::cout << std::endl;
}

static void brightness_transition(int speed) {
    int i = 0;
    while (i != 100) {
        i += speed;
        randowblimp(0, 100, i);
    }

    while (i != 0) {
        i -= speed;
        randowblimp(0, 100, i);
    }
}

static void limparTela() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coordScreen = { 0, 0 };
    DWORD cCharsWritten;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD dwConSize;

    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        return;
    }

    dwConSize = csbi.dwSize.X * csbi.dwSize.Y;
    if (!FillConsoleOutputCharacter(hConsole, (TCHAR)' ', dwConSize, coordScreen, &cCharsWritten)) {
        return;
    }
    if (!SetConsoleCursorPosition(hConsole, coordScreen)) {
        return;
    }
}

std::string logo = R"(
  _  ___  ___              _     
 | |/ / |/ / |            | |    
 | ' /| ' /| |_ ___   ___ | |___ 
 |  ; |  ; | __/ _ \ / _ \| / __|
 | . \| . \| |_ (_) | (_) | \__ \
 |_|\_\_|\_\\__\___/ \___/|_|___/
  github - Obentemiller                              
                                 )";

std::string ascii_art0 = R"(
    ._________________.       
    |.---------------.|       
    ||               ||       
    ||    PRONTO!    ||        
    ||               ||       
    ||               ||       
    ||_______________||       
    /.-.-.-.-.-.-.-.-.\       
   /.-.-.-.-.-.-.-.-.-.\      
  /.-.-.-.-.-.-.-.-.-.-.\    
 /______/__________\___o_\
 \_______________________/                                                             
)";

std::string ascii_art1 = R"(
    ._________________.       
    |.---------------.|       
    ||               ||       
    ||   ATÉ MAIS!   ||        
    ||  Obentemiller ||       
    ||               ||       
    ||_______________||       
    /.-.-.-.-.-.-.-.-.\     
   /.-.-.-.-.-.-.-.-.-.\    
  /.-.-.-.-.-.-.-.-.-.-.\    
 /______/__________\___o_\
 \_______________________/                                                             
)";

std::string ascii_art2 = R"(
     _________________     
    /                  \      
   /                    \       
  /                      \      
 /_____________obrigado __\     
 |------------------------|
 \________________________/                                                             
)";


std::vector<std::wstring> GetFilesInDirectory(const std::wstring& directoryPath)
{
    std::vector<std::wstring> files;

    // Prepara a máscara de busca
    std::wstring searchPath = directoryPath + L"\\*";

    // Inicializa a estrutura de pesquisa
    WIN32_FIND_DATA findData;
    HANDLE hFind = FindFirstFile(searchPath.c_str(), &findData);

    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            // Ignora entradas especiais de diretório
            if (wcscmp(findData.cFileName, L".") != 0 && wcscmp(findData.cFileName, L"..") != 0)
            {
                // Verifica se é um arquivo
                if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                {
                    files.push_back(directoryPath + L"\\" + findData.cFileName);
                }
            }
        } while (FindNextFile(hFind, &findData) != 0);

        // Fecha o identificador de pesquisa
        FindClose(hFind);
    }

    return files;
}

std::wstring GetFileExtension(const std::wstring& filePath)
{
    size_t dotPos = filePath.find_last_of(L'.');
    if (dotPos != std::wstring::npos && dotPos != filePath.size() - 1)
    {
        return filePath.substr(dotPos + 1);
    }
    return L"";
}

bool CreateDirectoryIfNotExists(const std::wstring& directory)
{
    if (!CreateDirectory(directory.c_str(), nullptr))
    {
        DWORD error = GetLastError();
        if (error != ERROR_ALREADY_EXISTS)
        {
            std::wcerr << L"Erro ao criar o diretório: " << directory << std::endl;
            return false;
        }
    }
    return true;
}

bool MoveFileToDirectory(const std::wstring& filePath, const std::wstring& directory)
{
    std::wstring destination = directory + L"\\" + filePath.substr(filePath.find_last_of(L"\\") + 1);
    if (!MoveFile(filePath.c_str(), destination.c_str()))
    {
        std::wcerr << L"Erro ao mover o arquivo: " << filePath << std::endl;
        return false;
    }
    return true;
}

bool OrganizeFilesByFormat(const std::wstring& directory)
{
    // Obtém os arquivos no diretório especificado
    std::vector<std::wstring> files = GetFilesInDirectory(directory);

    // Organiza os arquivos por formato
    std::map<std::wstring, std::vector<std::wstring>> filesByExtension;
    for (const auto& file : files)
    {
        std::wstring extension = GetFileExtension(file);
        filesByExtension[extension].push_back(file);
    }

    // Cria uma pasta para cada formato e move os arquivos correspondentes
    for (const auto& entry : filesByExtension)
    {
        std::wstring formatDirectory = directory + L"\\" + entry.first;
        if (CreateDirectoryIfNotExists(formatDirectory))
        {
            for (const auto& file : entry.second)
            {
                if (!MoveFileToDirectory(file, formatDirectory))
                {
                    // Se ocorrer um erro ao mover um arquivo, retorne false
                    return false;
                }
            }
        }
        else
        {
            // Se ocorrer um erro ao criar uma pasta para um formato, retorne false
            return false;
        }
    }

    return true;
}

std::wstring OpenDirectoryDialog(HWND hwndOwner)
{
    std::wstring selectedDirectory;

    // Inicializa a estrutura de diálogo de item comum
    BROWSEINFO browseInfo = { 0 };
    browseInfo.hwndOwner = hwndOwner;
    browseInfo.lpszTitle = L"Selecione um diretório";
    browseInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

    // Exibe o diálogo de seleção de diretório
    LPITEMIDLIST pidl = SHBrowseForFolder(&browseInfo);
    if (pidl != nullptr)
    {
        wchar_t path[MAX_PATH];
        if (SHGetPathFromIDList(pidl, path))
        {
            selectedDirectory = path;
        }
        CoTaskMemFree(pidl); // Libera o identificador de item de lista
    }

    return selectedDirectory;
}


static void finalização() {
    Sleep(1000);
    limparTela();
    std::cout << ascii_art0 << std::endl;
    Sleep(1000);
    limparTela();
    std::cout << ascii_art1 << std::endl;
    Sleep(1000);
    limparTela();
    std::cout << ascii_art2 << std::endl;
}

static void renova_interface() {
    limparTela();
    std::cout << logo << std::endl;
}

static char intrografic() {
    char in = ' ';

    while (in != 'q') {
        limparTela();
        std::cout << logo << std::endl;
        std::cout << "  <Pressione 'q' para continuar>" << std::endl;
        in = _getch();
    }

    brightness_transition(1);
    return(in);
}

static char selectgrafic() {
    char in = ' ';
    bool modo = false;
    while (in != 'q' && in != 27) {
        limparTela();
        std::cout << logo << std::endl;
        std::cout << "     selecione o modo " << (modo ? "<file organizer>" : "<software upgrade>") << std::endl;
        std::cout << "  <'q' para continuar>" << "  <'esc' para voltar>" << std::endl;
        in = _getch();
        if (in == 77 || in == 75) {
            modo = !modo;
            S_DAF = modo;
        }
    }

    brightness_transition(1);
    return(in);
}

static void software_upgrade() {
    limparTela();

    std::cout << logo << std::endl;
    std::cout << "program upgrade" << std::endl;
    system("winget upgrade --all --include-unknown");

    Sleep(1000);
    brightness_transition(1);
    limparTela();
    finalização();
}

static void file_organizer() {
    limparTela();

    std::cout << logo << std::endl;
    
    std::wstring selectedDirectory = OpenDirectoryDialog(nullptr);

    if (!selectedDirectory.empty())
    {
        std::wcout << L"Diretório selecionado: " << selectedDirectory << std::endl;

        if (OrganizeFilesByFormat(selectedDirectory))
        {
            std::wcout << L"Organização concluída." << std::endl;
        }
        else
        {
            std::wcerr << L"Erro ao organizar os arquivos." << std::endl;
        }
    }
    else
    {
        std::wcerr << L"Nenhum diretório selecionado." << std::endl;
    }

    Sleep(1000);
    brightness_transition(1);
    limparTela();
    finalização();
}

int main() {
inicio:
    intrografic();

    char selection = selectgrafic();
    if (selection == 'q') {
        switch (S_DAF) {
        case 0:
            software_upgrade();
            return 0;
        case 1:
            file_organizer();
            return 0;
        }
    }
    else if (selection == 27) {
        goto inicio;
    }

    return 0;
}
