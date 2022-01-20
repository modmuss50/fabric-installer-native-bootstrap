#include "Bootstrap.h"
#include "utils/SystemHelper.h"

int main() {
    const auto systemHelper = std::make_shared<Bootstrap::Utils::SystemHelper>();

    auto bs = Bootstrap::Bootstrap(systemHelper);
    bs.launch();

    return 0;
}
