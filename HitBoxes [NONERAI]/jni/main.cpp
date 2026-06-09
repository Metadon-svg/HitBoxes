#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>

// Подключаем ImGui и его OpenGL3 бэкенд
#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "Utils.h"

using namespace std;

// --- ХИТБОКСЫ (Оффсеты) ---
#if defined(__aarch64__)
    uintptr_t HEAD = 0x141DF68;
    uintptr_t TORSO_1 = HEAD + 0x20;
    uintptr_t TORSO_2 = TORSO_1 + 0x20;
    uintptr_t MID = TORSO_2 + 0x20;
    uintptr_t LEFTARM = MID + 0x20;
    uintptr_t RIGHTARM = LEFTARM + 0x20;
    uintptr_t LEFTLEG_1 = RIGHTARM + 0x20;
    uintptr_t RIGHTLEG_1 = LEFTLEG_1 + 0x20;
    uintptr_t LEFTLEG_2 = RIGHTLEG_1 + 0x20;
    uintptr_t RIGHTLEG_2 = LEFTLEG_2 + 0x20;
#else
    uintptr_t HEAD = 0x141DF68;
    uintptr_t TORSO_1 = HEAD + 0x18;
    uintptr_t TORSO_2 = TORSO_1 + 0x18;
    uintptr_t MID = TORSO_2 + 0x18;
    uintptr_t LEFTARM = MID + 0x18;
    uintptr_t RIGHTARM = LEFTARM + 0x18;
    uintptr_t LEFTLEG_1 = RIGHTARM + 0x18;
    uintptr_t RIGHTLEG_1 = LEFTLEG_1 + 0x18;
    uintptr_t LEFTLEG_2 = RIGHTLEG_1 + 0x18;
    uintptr_t RIGHTLEG_2 = LEFTLEG_2 + 0x18;
#endif

#define libName "libblackrussia-client.so"

// Глобальные переменные управления
float MultiplyValue = 1.5f;
float OldMultiplyValue = 1.5f;
bool isLibraryReady = false;
bool ImGui_Initialized = false;

// Функция записи в память
void ApplyHitboxChanges() {
    if (!isLibraryReady) return;
    Utils::WriteMemory<float>(getAbsoluteAddress(libName, HEAD), 0.15f * MultiplyValue);
    Utils::WriteMemory<float>(getAbsoluteAddress(libName, TORSO_1), 0.2f * MultiplyValue);
    Utils::WriteMemory<float>(getAbsoluteAddress(libName, TORSO_2), 0.25f * MultiplyValue);
    Utils::WriteMemory<float>(getAbsoluteAddress(libName, MID), 0.25f * MultiplyValue);
    Utils::WriteMemory<float>(getAbsoluteAddress(libName, LEFTARM), 0.16f * MultiplyValue);
    Utils::WriteMemory<float>(getAbsoluteAddress(libName, RIGHTARM), 0.16f * MultiplyValue);
    Utils::WriteMemory<float>(getAbsoluteAddress(libName, LEFTLEG_1), 0.2f * MultiplyValue);
    Utils::WriteMemory<float>(getAbsoluteAddress(libName, RIGHTLEG_1), 0.2f * MultiplyValue);
    Utils::WriteMemory<float>(getAbsoluteAddress(libName, LEFTLEG_2), 0.15f * MultiplyValue);
    Utils::WriteMemory<float>(getAbsoluteAddress(libName, RIGHTLEG_2), 0.15f * MultiplyValue);
}

// --- ОТРИСОВКА ИНТЕРФЕЙСА (МЕНЮ) ---
void DrawMenu() {
    ImGui::SetNextWindowSize(ImVec2(350, 120), ImGuiCond_FirstUseEver);
    
    // Стильное минималистичное окно
    ImGui::Begin("HitBox Controller", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Text("Настройки размера персонажей");
    ImGui::Separator();

    // Слайдер изменения размера хитбоксов
    if (ImGui::SliderFloat("Множитель", &MultiplyValue, 1.0f, 5.0f, "X: %.2f")) {
        if (MultiplyValue != OldMultiplyValue) {
            ApplyHitboxChanges();
            OldMultiplyValue = MultiplyValue;
        }
    }

    ImGui::End();
}

// --- ХУК ГРАФИКИ (EGL / OpenGL) ---
// Указатель на оригинальную функцию eglSwapBuffers игры
EGLBoolean (*old_eglSwapBuffers)(EGLDisplay dpy, EGLSurface surface) = nullptr;

// Наша кастомная функция, которая вызывается при каждом кадре игры
EGLBoolean hook_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    if (!ImGui_Initialized) {
        // Инициализируем контекст ImGui при первом кадре
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        
        // Настройка дефолтного размера шрифта под мобильные экраны
        io.FontGlobalScale = 2.5f; 

        // Инициализируем бэкенд OpenGL3
        ImGui_ImplOpenGL3_Init("#version 300 es");
        ImGui_Initialized = true;
    }

    // Создаем новый кадр интерфейса
    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    // Рисуем наше меню
    if (isLibraryReady) {
        DrawMenu();
    }

    // Рендерим ImGui поверх игры
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Возвращаем управление оригинальной функции игры, чтобы кадр отобразился
    return old_eglSwapBuffers(dpy, surface);
}

// Главный поток мода
void *main_thread(void *) {
    // Ждем загрузки целевой библиотеки игры
    do { sleep(1); } while (!isLibraryLoaded(libName));
    isLibraryReady = true;
    ApplyHitboxChanges();

    // Сюда нужно добавить вызов твоей библиотеки хуков (например, Dobby или Substrate),
    // чтобы перенаправить оригинальный eglSwapBuffers на наш hook_eglSwapBuffers.
    // Пример для Dobby Hook:
    // DobbyHook((void *)eglSwapBuffers, (void *)hook_eglSwapBuffers, (void **)&old_eglSwapBuffers);

    return nullptr;
}

// Инициализация при инжекте
__attribute__((constructor)) void _init(){
    pthread_t ptid;
    pthread_create(&ptid, NULL, main_thread, NULL);
}
