#include <iservice_registry.h>
#include "system_ability_mock.h"

namespace OHOS {

SystemAbilityManagerClient& SystemAbilityManagerClient::GetInstance()
{
    static SystemAbilityManagerClient instance;
    return instance;
}

sptr<ISystemAbilityManager> SystemAbilityManagerClient::GetSystemAbilityManager()
{
    if (g_mockSamgrReturnNull) {
        return nullptr;
    }
    return nullptr;
}

} // namespace OHOS

bool g_mockSamgrReturnNull = true;
