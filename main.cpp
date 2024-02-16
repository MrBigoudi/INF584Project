#include <cstdlib>
#include <iostream>

#include "applicationTest.hpp"

int main(int argc[[maybe_unused]], char* argv[[maybe_unused]][]){

    Application app{};

    app.run();

    exit(EXIT_SUCCESS);
}