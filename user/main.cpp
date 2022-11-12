// Generated C++ file by Il2CppInspector - http://www.djkaty.com - https://github.com/djkaty
// Custom injected code entry point

#include "pch-il2cpp.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <iostream>
#include "il2cpp-appdata.h"
#include "helpers.h"
#include <MinHook.h>
#include "../includes.h"
#include <map>

#if _WIN64
#pragma comment (lib, "libMinHook.x64.lib")
#else
#pragma comment (lib, "libMinHook.x86.lib")
#endif
using namespace app;

// Set the name of your log file here
extern const LPCWSTR LOG_FILE = L"il2cpp-log.txt";
FILE* fp = nullptr;
HMODULE hModule;
Il2CppImage* img;
Il2CppImage* img_camera;
bool noSpread_enabled = false;

// Unlimited Ammo
typedef void(__thiscall* weaponFire)(BulletWeapon* pthis, bool resetRecoilSimilarity, MethodInfo* method);
weaponFire origWeaponFire = nullptr; // original FP after Hook
weaponFire weaponFireTarget; // original FP before Hook - DO NOT CALL THIS!
typedef void(__thiscall* shotgunFire)(Shotgun* pthis, bool resetRecoilSimilarity, MethodInfo* method);
shotgunFire origShotgunFire = nullptr;
shotgunFire shotgunFireTarget;

void __stdcall HookWeaponFire(BulletWeapon* pthis, bool resetRecoilSimilarity, MethodInfo* method)
{
    int32_t* ammo_clip = &(pthis->fields).m_clip;
    *ammo_clip = 1338;
    return origWeaponFire(pthis, resetRecoilSimilarity, method);
}

void __stdcall HookShotgunFire(Shotgun* pthis, bool resetRecoilSimilarity, MethodInfo* method)
{
    int32_t* ammo_clip = &(pthis->fields)._.m_clip;
    *ammo_clip = 1338;
    return origShotgunFire(pthis, resetRecoilSimilarity, method);
}

// NoRecoil
typedef void(__thiscall* noRecoil)(Weapon* pthis, bool resetSimilarity, MethodInfo* method);
noRecoil origNoRecoil = nullptr;
noRecoil noRecoilTarget;

void __stdcall HookRecoil(Weapon* pthis, bool resetSimilarity, MethodInfo* method)
{

}

//Invincibility
typedef void(__thiscall* invincibility)(Dam_PlayerDamageLocal* pthis, MethodInfo* method);
invincibility origInvincibility = nullptr;
invincibility invincibilityTarget;

void __stdcall HookHealth(Dam_PlayerDamageLocal* pthis, MethodInfo* method)
{
    float* health = &(pthis->fields)._._._Health_k__BackingField;
    *health = 25.0f;
    return origInvincibility(pthis, method);
}

//ESP
typedef void(__thiscall* updateEnemy)(EnemyAgent* pthis, MethodInfo* method);
updateEnemy origUpdateEnemy = nullptr;
updateEnemy updateEnemyTarget;
typedef DWORD64(__thiscall* getTransform)(Component* cthis);
typedef DWORD64(__cdecl* getCurrCamera)();
Il2CppClass* enemyAgentClass;
Il2CppMethodPointer enemyGetPositionPtr;
Il2CppMethodPointer enemyGetEyePositionPtr;
Il2CppClass* cameraClassPtr;
Il2CppMethodPointer currCameraMethodPtr;
Il2CppMethodPointer w2sMethodPtr;
std::map<EnemyAgent*, std::pair<Vector3, Vector3>> enemyMap;
bool esp_enabled = false;
bool esp_lastdraw = false;
                              //Feet    Head
void __stdcall HookUpdatEnemy(EnemyAgent* pthis, MethodInfo* method) //ToDo: Add EyePosition for Head, CurrPos is Feet
{
    auto it = enemyMap.find(pthis);
    if(it == enemyMap.end())
    {
        Vector3(__cdecl * getPosition)(void* pthis);
        getPosition = (Vector3(__cdecl*)(void*))(enemyGetPositionPtr);
        Vector3 currPosFeet = getPosition(reinterpret_cast<void*>(pthis));

        Vector3(__cdecl * getPositionHead)(void* pthis);
        getPositionHead = (Vector3(__cdecl*)(void*))(enemyGetEyePositionPtr);
        Vector3 currPosHead = getPositionHead(reinterpret_cast<void*>(pthis));
        //Get current Camera and get currPos by W2S call
        getCurrCamera currCameraCall = reinterpret_cast<getCurrCamera>(currCameraMethodPtr);
        DWORD64 currentCamera = currCameraCall();
        //Calc W2S
        Vector3(__cdecl * getW2S)(void* camThis, Vector3 position);
        getW2S = (Vector3(__cdecl*)(void*, Vector3))(w2sMethodPtr);
        Vector3 currCoordsFeet = getW2S((void*)currentCamera, currPosFeet);
        Vector3 currCoordsHead = getW2S((void*)currentCamera, currPosHead);
        
        enemyMap.insert(std::make_pair(pthis, std::make_pair(currCoordsFeet, currCoordsHead)));
    }
    return origUpdateEnemy(pthis, method);
}

typedef void(__thiscall* lateUpdate)(FPSCamera* fcthis, MethodInfo* method);
lateUpdate origLateUpdate = nullptr;
lateUpdate lateUpdateTarget;
void __stdcall HookLateUpdate(FPSCamera* fcthis, MethodInfo* method)
{
    std::map<EnemyAgent*, std::pair<Vector3, Vector3>>::iterator it;
    for (it = enemyMap.begin(); it != enemyMap.end(); )
    {
        if (it->first->fields.m_alive == false)
        {
            enemyMap.erase(it++);
            continue;
        }
        else
        {
            Vector3(__cdecl * getPosition)(void* pthis);
            getPosition = (Vector3(__cdecl*)(void*))(enemyGetPositionPtr);
            Vector3 currPosFeet = getPosition(reinterpret_cast<void*>(it->first));
            Vector3(__cdecl * getPositionHead)(void* pthis);
            getPositionHead = (Vector3(__cdecl*)(void*))(enemyGetEyePositionPtr);
            Vector3 currPosHead = getPositionHead(reinterpret_cast<void*>(it->first));
            //Get current Camera and get currPos by W2S call
            getCurrCamera currCameraCall = reinterpret_cast<getCurrCamera>(currCameraMethodPtr);
            DWORD64 currentCamera = currCameraCall();
            //Calc W2S
            Vector3(__cdecl * getW2S)(void* camThis, Vector3 position);
            getW2S = (Vector3(__cdecl*)(void*, Vector3))(w2sMethodPtr);
            Vector3 currCoordsFeet = getW2S((void*)currentCamera, currPosFeet);
            Vector3 currCoordsHead = getW2S((void*)currentCamera, currPosHead);
            it->second.first = currCoordsFeet;
            it->second.second = currCoordsHead;
            it++;
        }
    }
    /*if (spawn_item)
    {
        //auto myVector3 = (Vector3__Boxed*)il2cpp_object_new((Il2CppClass*)*Vector3__TypeInfo);
        //Vector3__ctor(myVector3, 1.0f, 2.0f, 3.0f, nullptr);
        Vector3 myVector = (fcthis->fields).lastPos;
        auto myQuaternion = (Quaternion__Boxed*)il2cpp_object_new((Il2CppClass*)*Quaternion__TypeInfo);
        Quaternion__ctor(myQuaternion, 1.0f, 1.0f, 1.0f, 1.0f, nullptr);
        Il2CppClass* itemSpawnerClass = il2cpp_class_from_name(img, "", "ItemSpawner");
        Il2CppMethodPointer spawnMethodPtr = il2cpp_class_get_method_from_name(itemSpawnerClass, "SpawnItem", 2)->methodPointer;
        typedef void(__thiscall* shotgunFire)(Shotgun* pthis, bool resetRecoilSimilarity, MethodInfo* method);
        Item(__cdecl * spawnItem)(void* fcthis, Vector3* myVector, Quaternion* myQuaternion, nullptr);
        getW2S = (Vector3(__cdecl*)(void*, Vector3))(w2sMethodPtr);
    }*/
    return origLateUpdate(fcthis, method);
}

void drawEnemies()
{
    //Draw only Enemies in Range
    std::map<EnemyAgent*, std::pair<Vector3, Vector3>>::iterator it;
    for (it = enemyMap.begin(); it != enemyMap.end();)
    {
        if (it->first->fields._.m_isBeingDestroyed == true)
        {
            enemyMap.erase(it++);
            continue;
        }
        ImVec2 enemyPointFeet = { it->second.first.x, ImGui::GetIO().DisplaySize.y - it->second.first.y };
        if (it->second.first.z > 0.5)
        {
            ImVec2 enemyPointHead = { it->second.second.x, ImGui::GetIO().DisplaySize.y - it->second.second.y };
            float h = enemyPointHead.y - enemyPointFeet.y;
            float w = h / 4.f;
            float l = enemyPointFeet.x - w;
            float r = enemyPointFeet.x + w;
            ImDrawList* DrawList = ImGui::GetBackgroundDrawList();
            //ImVec2 BottomOfScreen = { ImGui::GetIO().DisplaySize.x / 2, ImGui::GetIO().DisplaySize.y };
            //DrawList->AddLine(BottomOfScreen, enemyPoint, ImColor(255, 255, 255, 255), 2.0F);
            DrawList->AddRect(ImVec2(l, enemyPointHead.y), ImVec2(r, enemyPointFeet.y), ImColor(255, 255, 255, 255), 0.f, 0);
        }
        it++;
    }
    if (!esp_enabled)
    {
        esp_lastdraw = true;
    }
}

typedef void(__thiscall* updateSentryAmmo)(SentryGunInstance* sthis, int Ammo ,MethodInfo* method);
updateSentryAmmo origUpdateSentryAmmo = nullptr;
updateSentryAmmo updateSentryAmmoTarget;
void __stdcall HookSentryAmmo(SentryGunInstance* sthis, int Ammo, MethodInfo* method)
{
    //(sthis->fields).m_ammo = 1337;
    return origUpdateSentryAmmo(sthis, 1337, method);
}

//Deject
DWORD __stdcall EjectThread(LPVOID lpParameter) {
    FreeLibraryAndExitThread(hModule, 0);
    return 0;
}

void shutdown()
{
    MH_Uninitialize();
    std::cout << "Bye Bye!" << std::endl;
    Sleep(1000);
    if (fp != nullptr)
        fclose(fp);
    FreeConsole();
    CreateThread(0, 0, EjectThread, 0, 0, 0);
    return;
}

//Init Kiero Hook
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
Present oPresent;
HWND window = NULL;
WNDPROC oWndProc;
ID3D11Device* pDevice = NULL;
ID3D11DeviceContext* pContext = NULL;
ID3D11RenderTargetView* mainRenderTargetView;
static IDXGISwapChain* g_pSwapChain = NULL;

void InitImGui()
{
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
    ImGui_ImplWin32_Init(window);
    ImGui_ImplDX11_Init(pDevice, pContext);
}

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

    if (true && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
        return true;

    return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

bool init = false;
bool menu = false;

HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
    if (!init)
    {
        if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&pDevice)))
        {
            pDevice->GetImmediateContext(&pContext);
            DXGI_SWAP_CHAIN_DESC sd;
            pSwapChain->GetDesc(&sd);
            window = FindWindowA(0, "GTFO");//sd.OutputWindow;
            ID3D11Texture2D* pBackBuffer;
            g_pSwapChain = pSwapChain;
            g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
            pDevice->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView);
            pBackBuffer->Release();
            oWndProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WndProc);
            InitImGui();
            init = true;
        }

        else
            return oPresent(pSwapChain, SyncInterval, Flags);
    }
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    //ImGui::Begin("ImGui Window");
    //ImGui::End();
    drawEnemies(); //ToDo: Disable Drawings when ESP turned Off!
    ImGui::Render();

    pContext->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    return oPresent(pSwapChain, SyncInterval, Flags);
}

bool initD3D11Hook()
{
    bool init_hook = false;
    do
    {
        if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success)
        {
            kiero::bind(8, (void**)&oPresent, hkPresent);
            init_hook = true;
        }
    } while (!init_hook);
    return TRUE;
}


// Custom injected code entry point
void Run(HMODULE hMODULE)
{
    // Initialize thread data - DO NOT REMOVE
    il2cpp_thread_attach(il2cpp_domain_get());
    hModule = hMODULE;
    //il2cppi_new_console();
    AllocConsole();
    freopen_s(&fp, "CONOUT$", "w", stdout);
    std::cout << "Welcome to Hevi Hook!" << std::endl; 
    Il2CppDomain* myDomain = il2cpp_domain_get();
    const Il2CppAssembly* myAssembly = il2cpp_domain_assembly_open(myDomain, "Modules-ASM");
    const Il2CppAssembly* cameraAssembly = il2cpp_domain_assembly_open(myDomain, "UnityEngine.CoreModule");
    img = myAssembly->image;
    img_camera = cameraAssembly->image;
    MH_STATUS status = MH_Initialize();
    if (status != MH_OK)
    {
        std::string sStatus = MH_StatusToString(status);
        std::cout << "Could not Initialize MinHook!" << std::endl;
        return;
    }
    initD3D11Hook();
    std::cout << "Preparations Done!" << std::endl;

    //Unlimited Ammo Method Pointers and Hooks
    Il2CppClass* bulletWeaponClassPtr = il2cpp_class_from_name(myAssembly->image, "Gear", "BulletWeapon");
    Il2CppClass* shotgunClassPtr = il2cpp_class_from_name(myAssembly->image, "Gear", "Shotgun");
    Il2CppMethodPointer bulletWeaponFirePtr = il2cpp_class_get_method_from_name(bulletWeaponClassPtr, "Fire", 1)->methodPointer;
    Il2CppMethodPointer shotgunFirePtr = il2cpp_class_get_method_from_name(shotgunClassPtr, "Fire", 1)->methodPointer;
    weaponFireTarget = reinterpret_cast<weaponFire>(bulletWeaponFirePtr);
    shotgunFireTarget = reinterpret_cast<shotgunFire>(shotgunFirePtr);
    if (MH_CreateHook(reinterpret_cast<void**>(weaponFireTarget), &HookWeaponFire, reinterpret_cast<void**>(&origWeaponFire)) != MH_OK)
    {
        std::cout << "Weapon Hook could not be created!" << std::endl;
        return;
    }
    if (MH_CreateHook(reinterpret_cast<void**>(shotgunFireTarget), &HookShotgunFire, reinterpret_cast<void**>(&origShotgunFire)) != MH_OK)
    {
        std::cout << "Shotgun Hook could not be created!" << std::endl;
        return;
    }


    //No Recoil Method Pointer
    Il2CppClass* recoilClassPtr = il2cpp_class_from_name(myAssembly->image, "", "Weapon");
    Il2CppMethodPointer recoilMethodPtr = il2cpp_class_get_method_from_name(recoilClassPtr, "ApplyRecoil", 1)->methodPointer;
    noRecoilTarget = reinterpret_cast<noRecoil>(recoilMethodPtr);
    if (MH_CreateHook(reinterpret_cast<void**>(noRecoilTarget), &HookRecoil, NULL) != MH_OK)
    {
        std::cout << "Recoil Hook could not be created!" << std::endl;
        return;
    }

    //No Spread Prep
    BYTE oldBytes[100]{ 0 };
    BYTE oldBytesShotty[100]{ 0 };
    DWORD64 dllBase = (DWORD64)GetModuleHandle(L"GameAssembly.dll");
    DWORD64 noSpreadAddress = dllBase + 0x905900;
    DWORD64 noSpreadAddressShotty = dllBase + 0x7ECEB8;

    //Invincibility Method Pointer
    Il2CppClass* damageClassPtr = il2cpp_class_from_name(myAssembly->image, "", "Dam_PlayerDamageLocal");
    Il2CppMethodPointer dmgClassUpdatePtr = il2cpp_class_get_method_from_name(damageClassPtr, "Update", 0)->methodPointer;
    invincibilityTarget = reinterpret_cast<invincibility>(dmgClassUpdatePtr);
    if (MH_CreateHook(reinterpret_cast<void**>(invincibilityTarget), &HookHealth, reinterpret_cast<void**>(&origInvincibility)) != MH_OK)
    {
        std::cout << "Health Hook could not be created!" << std::endl;
        return;
    }

    //Update Enemy Method Pointer
    Il2CppClass* enemyClassPtr = il2cpp_class_from_name(myAssembly->image, "Enemies", "EnemyAgent");
    Il2CppMethodPointer updateEnemyMethodPtr = il2cpp_class_get_method_from_name(enemyClassPtr, "UpdateEnemyAgent", 0)->methodPointer;
    updateEnemyTarget = reinterpret_cast<updateEnemy>(updateEnemyMethodPtr);
    if (MH_CreateHook(reinterpret_cast<void**>(updateEnemyTarget), &HookUpdatEnemy, reinterpret_cast<void**>(&origUpdateEnemy)) != MH_OK)
    {
        std::cout << "Enemy Update Hook could not be created!" << std::endl;
        return;
    }

    //Hook FPSCamera LateUpdate
    Il2CppClass* lateUpdateClassPtr = il2cpp_class_from_name(myAssembly->image, "", "FPSCamera");
    Il2CppMethodPointer lateUpdateMethodPtr = il2cpp_class_get_method_from_name(lateUpdateClassPtr, "LateUpdate", 0)->methodPointer;
    lateUpdateTarget = reinterpret_cast<lateUpdate>(lateUpdateMethodPtr);
    if (MH_CreateHook(reinterpret_cast<void**>(lateUpdateTarget), &HookLateUpdate, reinterpret_cast<void**>(&origLateUpdate)) != MH_OK)
    {
        std::cout << "Late Update Hook could not be created!" << std::endl;
        return;
    }
    if (MH_EnableHook(reinterpret_cast<void**>(lateUpdateTarget)) != MH_OK)
    {
        std::cout << "LateUpdate Hook could not be enabled!" << std::endl;
        return;
    }
    // Function Calls for ESP:
    enemyAgentClass = il2cpp_class_from_name(img, "Enemies", "EnemyAgent");
    enemyGetPositionPtr = il2cpp_class_get_method_from_name(enemyAgentClass, "get_Position", 0)->methodPointer;
    cameraClassPtr = il2cpp_class_from_name(img_camera, "UnityEngine", "Camera");
    currCameraMethodPtr = il2cpp_class_get_method_from_name(cameraClassPtr, "get_current", 0)->methodPointer;
    w2sMethodPtr = il2cpp_class_get_method_from_name(cameraClassPtr, "WorldToScreenPoint", 1)->methodPointer;
    enemyGetEyePositionPtr = il2cpp_class_get_method_from_name(enemyAgentClass, "get_EyePosition", 0)->methodPointer;

    //Hook SentryGunInstance 
    Il2CppClass* sentryGunInstanceClassPtr = il2cpp_class_from_name(myAssembly->image, "", "SentryGunInstance_Firing_Bullets");
    Il2CppMethodPointer sentryGunUpdateAmmoMethodPtr = il2cpp_class_get_method_from_name(sentryGunInstanceClassPtr, "UpdateAmmo", 1)->methodPointer;
    updateSentryAmmoTarget = reinterpret_cast<updateSentryAmmo>(sentryGunUpdateAmmoMethodPtr);
    if (MH_CreateHook(reinterpret_cast<void**>(updateSentryAmmoTarget), &HookSentryAmmo, reinterpret_cast<void**>(&origUpdateSentryAmmo)) != MH_OK)
    {
        std::cout << "Sentry Hook could not be created!" << std::endl;
        return;
    }

    bool unlimitedAmmo_enabled = false;
    bool noRecoil_enabled = false;
    bool invincibility_enabled = false;
    bool unlimitedSentryAmmo_enabled = false;


    while (true) 
    {
        Sleep(50);
        if (GetAsyncKeyState(VK_F1) & 1)
        {
            unlimitedAmmo_enabled = !unlimitedAmmo_enabled;
            if (unlimitedAmmo_enabled)
            {
                std::cout << "Unlimited Ammo enabled!" << std::endl;
                if (MH_EnableHook(reinterpret_cast<void**>(weaponFireTarget)) != MH_OK)
                {
                    std::cout << "Weapon Hook could not be enabled!" << std::endl;
                    return;
                }
                if (MH_EnableHook(reinterpret_cast<void**>(shotgunFireTarget)) != MH_OK)
                {
                    std::cout << "Shotgun Hook could not be enabled!" << std::endl;
                    return;
                }
            }
            else
            {
                std::cout << "Unlimited Ammo disabled!" << std::endl;
                if (MH_DisableHook(reinterpret_cast<void**>(weaponFireTarget)) != MH_OK)
                {
                    std::cout << "Weapon Hook could not be disabeld!" << std::endl;
                    return;
                }
                if (MH_DisableHook(reinterpret_cast<void**>(shotgunFireTarget)) != MH_OK)
                {
                    std::cout << "Shotgun Hook could not be disabeld!" << std::endl;
                    return;
                }
            }
        }
        if (GetAsyncKeyState(VK_F2) & 1)
        {
            noRecoil_enabled = !noRecoil_enabled;
            if(noRecoil_enabled)
            {
                std::cout << "NoRecoil enabled!" << std::endl;
                if (MH_EnableHook(reinterpret_cast<void**>(noRecoilTarget)) != MH_OK)
                {
                    std::cout << "Recoil Hook could not be enabled!" << std::endl;
                    return;
                }
            }
            else
            {
                std::cout << "NoRecoil disabled!" << std::endl;
                if (MH_DisableHook(reinterpret_cast<void**>(noRecoilTarget)) != MH_OK)
                {
                    std::cout << "Recoil Hook could not be disabeld!" << std::endl;
                    return;
                }
            }
        }
        if (GetAsyncKeyState(VK_F3) & 1)
        {
            noSpread_enabled = !noSpread_enabled;
            if (noSpread_enabled)
            {
                std::cout << "NoSpread enabled!" << std::endl;
                DWORD d, ds;
                VirtualProtect((LPVOID)noSpreadAddress, 5, PAGE_EXECUTE_READWRITE, &d);
                ReadProcessMemory(GetCurrentProcess(), (LPVOID)(noSpreadAddress), &oldBytes, 5, NULL);
                WriteProcessMemory(GetCurrentProcess(), (LPVOID)(noSpreadAddress), "\x90\x90\x90\x90\x90", 5, NULL);
                VirtualProtect((LPVOID)noSpreadAddress, 5, d, &ds);

                VirtualProtect((LPVOID)noSpreadAddressShotty, 5, PAGE_EXECUTE_READWRITE, &d);
                ReadProcessMemory(GetCurrentProcess(), (LPVOID)(noSpreadAddressShotty), &oldBytesShotty, 5, NULL);
                WriteProcessMemory(GetCurrentProcess(), (LPVOID)(noSpreadAddressShotty), "\x90\x90\x90\x90\x90", 5, NULL);
                VirtualProtect((LPVOID)noSpreadAddressShotty, 5, d, &ds);
            }
            else
            {
                std::cout << "NoSpread disabled!" << std::endl;
                DWORD d, ds;
                VirtualProtect((LPVOID)noSpreadAddress, 5, PAGE_EXECUTE_READWRITE, &d);
                WriteProcessMemory(GetCurrentProcess(), (LPVOID)(noSpreadAddress), (BYTE*)&oldBytes, 5, NULL);
                VirtualProtect((LPVOID)noSpreadAddress, 5, d, &ds);

                VirtualProtect((LPVOID)noSpreadAddressShotty, 5, PAGE_EXECUTE_READWRITE, &d);
                WriteProcessMemory(GetCurrentProcess(), (LPVOID)(noSpreadAddressShotty), (BYTE*)&oldBytesShotty, 5, NULL);
                VirtualProtect((LPVOID)noSpreadAddressShotty, 5, d, &ds);
            }
        }
        if (GetAsyncKeyState(VK_F4) & 1)
        {
            invincibility_enabled = !invincibility_enabled;
            if (invincibility_enabled)
            {
                std::cout << "You're John Wick now!" << std::endl;
                if (MH_EnableHook(reinterpret_cast<void**>(invincibilityTarget)) != MH_OK)
                {
                    std::cout << "Recoil Hook could not be enabled!" << std::endl;
                    return;
                }
            }
            else
            {
                std::cout << "Lmao you're back to being a Goofy" << std::endl;
                if (MH_DisableHook(reinterpret_cast<void**>(invincibilityTarget)) != MH_OK)
                {
                    std::cout << "Recoil Hook could not be disabeld!" << std::endl;
                    return;
                }
            }
        }
        if (GetAsyncKeyState(VK_F5) & 1)
        {
            esp_enabled = !esp_enabled;
            if (esp_enabled)
            {
                std::cout << "Nice!" << std::endl;
                if (MH_EnableHook(reinterpret_cast<void**>(updateEnemyTarget)) != MH_OK)
                {
                    std::cout << "Enemy Hook could not be enabled!" << std::endl;
                    return;
                }
            }
            else
            {
                std::cout << "Not Nice!" << std::endl;
                if (MH_DisableHook(reinterpret_cast<void**>(updateEnemyTarget)) != MH_OK)
                {
                    std::cout << "Enemy Hook could not be disabeld!" << std::endl;
                    return;
                }
            }
        }
        if (GetAsyncKeyState(VK_F6) & 1)
        {
            unlimitedSentryAmmo_enabled = !unlimitedSentryAmmo_enabled;
            if (unlimitedSentryAmmo_enabled)
            {
                std::cout << "Unlimited Sentry Ammo enabled!" << std::endl;
                if (MH_EnableHook(reinterpret_cast<void**>(updateSentryAmmoTarget)) != MH_OK)
                {
                    std::cout << "Sentry Hook could not be enabled!" << std::endl;
                    return;
                }
            }
            else
            {
                std::cout << "Unlimited Sentry Ammo disabled!" << std::endl;
                if (MH_DisableHook(reinterpret_cast<void**>(updateSentryAmmoTarget)) != MH_OK)
                {
                    std::cout << "Sentry Hook could not be disabeld!" << std::endl;
                    return;
                }
            }
        }
        if (GetAsyncKeyState(VK_F11) & 1)
        {
            break;
        }
        if (esp_enabled == false && esp_lastdraw)
        {
            enemyMap.clear();
            esp_lastdraw = false;
        }
    }

    shutdown();
    return;
}