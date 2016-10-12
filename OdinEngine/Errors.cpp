#include "Errors.h"

#include <cstdlib>

#include <iostream>
#include <SDL/SDL.h>
#include <FMOD/fmod.h>

namespace OdinEngine {
    void fatalError(std::string errorString)
    {
        std::cout << errorString << std::endl;
        std::cout << "Enter any key to quit...";
        int tmp;
        std::cin >> tmp;
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    int fmodErrorCheck(FMOD_RESULT result) {
        if (result != FMOD_OK) {
            switch (result) {
            case FMOD_ERR_FILE_NOTFOUND:
                std::cout << "Fmod: File not found.\n";
                break;
            case FMOD_ERR_FORMAT:
                std::cout << "Fmod: Unsupported format.\n";
                break;
            default:
                std::cout << "Fmod error result: " << result << std::endl;
                break;
            }
            return 1; //fail
        }
        return 0; //okay
    }
}