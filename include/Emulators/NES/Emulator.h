#pragma once
#include <Engine/Core/Game/Game.h>

#include "./Bus/Bus.h"
#include "./CPU/6502.h"
#include "./PPU/2C02.h"
#include "./SoundPacer.h"
#include "../Controller.h"

#include <iostream>
#include <sstream>
#include <list>
#include <filesystem>

namespace UnifiedEmulation {
    namespace NES {

        enum DebugConfigVals{
            Debug_None,
            Debug_Status,
            Debug_Ram,
            Debug_Code,
            Debug_Paletts,
            Debug_Sprites,
            Debug_Audio            
        };

        struct DebugConfig{
            DebugConfigVals Section1;
            DebugConfigVals Section2;
            DebugConfigVals Section3;
        };

        class NESEmulator{
        public:
            bool DebugMode = false;
            DebugConfig Debug;
        public:
            NESEmulator(Game* game, string FontFileLocation)
                : PaletteSelector(ivec2(28, 10), vec3(1)), ImagePixel(ivec2(360, 240)), RomSelector(ivec2(5, 5), vec3(1)), font(FontFileLocation.c_str(), 8)
            {
                this->system = new Bus;
                this->game = game;
                this->PlayAudio = new bool(false);

                this->Button_Pressed = 0;
                this->lastKey = Key_N;

                //Load Cart
                if(GetRoms().size() > 0){
                    this->cart = std::make_shared<Cartridge>(GetRoms()[CurrentRom].string());

                      //Load To NES
                    this->system->insertCartridge(cart);
                }

                //Extract dissassembly
                mapAsm = system->cpu.disassemble(0x0000, 0xFFFF);

                //Reset
                this->system->reset();

                //Create Palette Selector Image
                for (int y = 0; y < 6; y++){
                    for (int x = 0; x < 24; x++){
                        this->PaletteSelector.SetPixel(ivec2(x + 2, y + 2), vec4(0));
                    }
                }

                for (int i = 0; i < 4; i++)
                {			
                    for (int j = 0; j < 120; j++)
                        this->audioV[i].push_back(0);
                }

                this->Audios = new PixelImage(ivec2(120, 120));

                this->emulatorPointer = this;
                this->system->SetSampleFrequency(44100);
                
                this->SoundDriver.InitialiseAudio(44100, 1, 8, 512);
                this->SoundDriver.SetUserSynthFunction(SoundOut);
            }

            ~NESEmulator(){
                delete this->system;
                if(PlayAudio)
                    this->SoundDriver.DestroyAudio();
            }
        
        public:
            Bus* system;

            std::shared_ptr<Cartridge> cart;
            EmulationSound SoundDriver;
            
            static NESEmulator* emulatorPointer;

        private:
            Game* game;

            PixelImage PaletteSelector;
            PixelImage ImagePixel;
            PixelImage RomSelector;

            PixelImage* Audios;
            vector<vector<PixelImage*>> Pals;

            Font font;

        public:
            vector<ControllerButtons> Controllers = {ControllerButtons{}, ControllerButtons{}};

            Keys lastKey;

        public:
            bool emulationRun = false;
            bool Button_Pressed;

            bool RomHotSwap = false;

            bool ToggleExtraController = false;

            static bool* PlayAudio;

        public:
            int ClockSpeed = 0;
            int CurrentFrame = 0;

            int CurrentRom = 0;

            int cartChangeInterval = 0;

            uint8_t nSelectedPalette = 0x00;
            int nSwatchSize = 6;

        public:
            float fResidualTime = 0.0f;

            float current_scale = 1;

        public:

            std::map<uint16_t, std::string> mapAsm;

            list<uint16_t> audioV[4];

        private:
            std::vector<std::filesystem::path> GetRoms(){
                namespace stdfs = std::filesystem ;

                std::vector<std::filesystem::path> filenames ;
                
                const stdfs::directory_iterator end{} ;
                
                for( stdfs::directory_iterator iter{"./rsc/ROMS/NES/"} ; iter != end ; ++iter )
                {
                    if( stdfs::is_regular_file(*iter) )
                        filenames.push_back( iter->path()) ;
                }

                return filenames;
            }

            std::string hex(uint32_t n, uint8_t d)
            {
                std::string s(d, '0');
                for (int i = d - 1; i >= 0; i--, n >>= 4)
                    s[i] = "0123456789ABCDEF"[n & 0xF];
                return s;
            };

        private: //Drawing
            void DrawImage(PixelImage& i, vec2 pos, string name){
                Image* ImageFound = nullptr;
                bool found = false;
                for(Image* l: game->ActiveScene.UI.Images){
                    if(l->name == name){
                        ImageFound = l;
                        found = true;
                        //cout << l->Name << " " << Identifier << "\n";
                        break;
                    }
                }

                pos.y = 475-pos.y;

                if(!found){
                    Image* ImageNew = new Image(&i, pos);
                    if(name == "MainView"){
                        ImageNew->scale = ivec2(2);
                    }
                    ImageNew->name = name;
                    ImageNew->scaleConstant = true;

                    game->ActiveScene.UI.NewImage(ImageNew);
                }
                else{
                    ImageFound->Position = pos;
                }
            }

            void DrawString(vec2 Pos, string text, vec3 Color, string Identifier){
                Label* LabelFound = nullptr;
                bool found = false;
                for(Label* l: game->ActiveScene.UI.Labels){
                    if(l->Name == Identifier){
                        LabelFound = l;
                        found = true;
                        break;
                    }
                }

                Pos.y = 460-Pos.y;

                if(!found){
                    Label* LabelNew = new Label(font);
                    LabelNew->Text = text;
                    LabelNew->Name = Identifier;
                    LabelNew->Color = Color;
                    LabelNew->Position = Pos;

                    game->ActiveScene.UI.NewLabel(LabelNew);
                }
                else{
                    LabelFound->Color = Color;
                    LabelFound->Text = text;
                    LabelFound->Position = Pos;
                }
            }

        private: //Debug

            void DrawDebug(){
                vec2 Section1Pos = vec2(516, -7);
                vec2 Section2Pos = vec2(516, 153);
                vec2 Section3Pos = vec2(516, 323);

                switch (this->Debug.Section1)
                {
                case Debug_Status:
                    this->DrawCpu(Section1Pos.x, Section1Pos.y);
                    break;
                
                case Debug_Ram:
                    DrawRam(Section1Pos.x, Section1Pos.y, 0x0000, 16, 14);
                    break;

                case Debug_Code:
                    DrawCode(Section1Pos.x, Section1Pos.y, 16);
                    break;

                case Debug_Paletts:
                    for (int p = 0; p < 8; p++){
                        for(int s = 0; s < 4; s++){
                            vec3 color = this->system->ppu.GetColorFromPaletteRam(p, s);
                            color = vec3(color.x / 255.f, color.y / 255.f, color.z / 255.f);

                            if(this->Pals.size() < p + 1){
                                this->Pals.push_back(vector<PixelImage*>{});
                            }
                            if(this->Pals[p].size() < s + 1){
                                this->Pals[p].push_back(new PixelImage(ivec2(nSwatchSize, nSwatchSize), color));
                            }

                            this->Pals[p][s]->Configure(ivec2(nSwatchSize, nSwatchSize), color);
                            this->DrawImage(*this->Pals[p][s], ivec2(Section1Pos.x + p * (nSwatchSize * 5) + s * nSwatchSize, Section1Pos.y+15), ("Pal." + to_string(p) + "." + to_string(s)));
                        }
                    }

                    this->DrawImage(this->PaletteSelector, ivec2(Section1Pos.x + this->nSelectedPalette * (nSwatchSize * 5) - 2, Section1Pos.y+17), "PalSelector");

                    this->DrawImage(this->system->ppu.GetPatternTable(0, this->nSelectedPalette), ivec2(Section1Pos.x, Section1Pos.y+148), "PT1");
                    this->DrawImage(this->system->ppu.GetPatternTable(1, this->nSelectedPalette), ivec2(Section1Pos.x+130, Section1Pos.y+148), "PT2");
                    break;

                case Debug_Sprites:
                    for(int i = 0; i < 16; i++){
                        std::string s= hex(i, 2) + ": (" + std::to_string(system->ppu.pOAM[i * 4 + 3])
                            + ", " + std::to_string(system->ppu.pOAM[i * 3 + 0]) + ") "
                            + "ID: " + hex(system->ppu.pOAM[i*4 + 1], 2) +
                            + " AT: " + hex(system->ppu.pOAM[i * 4 + 2], 2);
                        DrawString(ivec2(Section1Pos.x, Section1Pos.y + i * 10), s, vec3(1), ("Sprites" + to_string(i)));
                    }
                    break;
                
                case Debug_Audio:
                    this->Audios->Configure(ivec2(250, 120), vec3(0));
                    //if (CurrentFrame % static_cast<int>(1 / Time.deltaTime) == 0){
                        this->audioV[0].pop_front();
                        this->audioV[0].push_back(this->system->apu.pulse1_visual);
                        this->audioV[1].pop_front();
                        this->audioV[1].push_back(this->system->apu.pulse2_visual);
                        this->audioV[2].pop_front();
                        this->audioV[2].push_back(this->system->apu.noise_visual);
                        this->audioV[3].pop_front();
                        this->audioV[3].push_back(this->system->apu.triangle_visual);
                    //}
                    DrawAudio(1, Section1Pos.x, 0);
                    DrawAudio(2, Section1Pos.x, 0);
                    DrawAudio(3, Section1Pos.x, 0);
                    DrawAudio(4, Section1Pos.x, 0);

                    this->DrawImage(*this->Audios, ivec2(Section1Pos.x, Section1Pos.y + 120), "Audio" + to_string(0));
                    break;

                case Debug_None:
                    break;
                }

                switch (this->Debug.Section2)
                {
                case Debug_Status:
                    this->DrawCpu(Section2Pos.x, Section2Pos.y);
                    break;
                
                case Debug_Ram:
                    DrawRam(Section2Pos.x, Section2Pos.y, 0x0000, 16, 14);
                    break;

                case Debug_Code:
                    DrawCode(Section2Pos.x, Section2Pos.y, 16);
                    break;

                case Debug_Paletts:
                    for (int p = 0; p < 8; p++){
                        for(int s = 0; s < 4; s++){
                            vec3 color = this->system->ppu.GetColorFromPaletteRam(p, s);
                            color = vec3(color.x / 255.f, color.y / 255.f, color.z / 255.f);

                            if(this->Pals.size() < p + 1){
                                this->Pals.push_back(vector<PixelImage*>{});
                            }
                            if(this->Pals[p].size() < s + 1){
                                this->Pals[p].push_back(new PixelImage(ivec2(nSwatchSize, nSwatchSize), color));
                            }

                            this->Pals[p][s]->Configure(ivec2(nSwatchSize, nSwatchSize), color);
                            this->DrawImage(*this->Pals[p][s], ivec2(Section2Pos.x + p * (nSwatchSize * 5) + s * nSwatchSize, Section2Pos.y+15), ("Pal." + to_string(p) + "." + to_string(s)));
                        }
                    }

                    this->DrawImage(this->PaletteSelector, ivec2(Section2Pos.x + this->nSelectedPalette * (nSwatchSize * 5) - 2, Section2Pos.y+17), "PalSelector");

                    this->DrawImage(this->system->ppu.GetPatternTable(0, this->nSelectedPalette), ivec2(Section2Pos.x, Section2Pos.y+148), "PT1");
                    this->DrawImage(this->system->ppu.GetPatternTable(1, this->nSelectedPalette), ivec2(Section2Pos.x+130, Section2Pos.y+148), "PT2");
                    break;

                case Debug_Sprites:
                    for(int i = 0; i < 16; i++){
                        std::string s= hex(i, 2) + ": (" + std::to_string(system->ppu.pOAM[i * 4 + 3])
                            + ", " + std::to_string(system->ppu.pOAM[i * 3 + 0]) + ") "
                            + "ID: " + hex(system->ppu.pOAM[i*4 + 1], 2) +
                            + " AT: " + hex(system->ppu.pOAM[i * 4 + 2], 2);
                        DrawString(ivec2(Section2Pos.x, Section2Pos.y + i * 10), s, vec3(1), ("Sprites" + to_string(i)));
                    }
                    break;

                case Debug_Audio:
                    this->Audios->Configure(ivec2(250, 120), vec3(0));
                    //if (CurrentFrame % static_cast<int>(1 / Time.deltaTime) == 0){
                        this->audioV[0].pop_front();
                        this->audioV[0].push_back(this->system->apu.pulse1_visual);
                        this->audioV[1].pop_front();
                        this->audioV[1].push_back(this->system->apu.pulse2_visual);
                        this->audioV[2].pop_front();
                        this->audioV[2].push_back(this->system->apu.noise_visual);
                        this->audioV[3].pop_front();
                        this->audioV[3].push_back(this->system->apu.triangle_visual);
                    //}
                    DrawAudio(1, Section2Pos.x, 0);
                    DrawAudio(2, Section2Pos.x, 0);
                    DrawAudio(3, Section2Pos.x, 0);
                    DrawAudio(4, Section2Pos.x, 0);

                    this->DrawImage(*this->Audios, ivec2(Section2Pos.x, Section2Pos.y + 120), "Audio" + to_string(0));
                    break;

                case Debug_None:
                    break;
                }

                switch (this->Debug.Section3)
                {
                case Debug_Status:
                    this->DrawCpu(Section3Pos.x, Section3Pos.y);
                    break;
                
                case Debug_Ram:
                    DrawRam(Section3Pos.x, Section3Pos.y, 0x0000, 16, 14);
                    break;

                case Debug_Code:
                    DrawCode(Section3Pos.x, Section3Pos.y, 16);
                    break;

                case Debug_Paletts:
                    for (int p = 0; p < 8; p++){
                        for(int s = 0; s < 4; s++){
                            vec3 color = this->system->ppu.GetColorFromPaletteRam(p, s);
                            color = vec3(color.x / 255.f, color.y / 255.f, color.z / 255.f);

                            if(this->Pals.size() < p + 1){
                                this->Pals.push_back(vector<PixelImage*>{});
                            }
                            if(this->Pals[p].size() < s + 1){
                                this->Pals[p].push_back(new PixelImage(ivec2(nSwatchSize, nSwatchSize), color));
                            }

                            this->Pals[p][s]->Configure(ivec2(nSwatchSize, nSwatchSize), color);
                            this->DrawImage(*this->Pals[p][s], ivec2(Section3Pos.x + p * (nSwatchSize * 5) + s * nSwatchSize, Section3Pos.y+15), ("Pal." + to_string(p) + "." + to_string(s)));
                        }
                    }

                    this->DrawImage(this->PaletteSelector, ivec2(Section3Pos.x + this->nSelectedPalette * (nSwatchSize * 5) - 2, Section3Pos.y+17), "PalSelector");

                    this->DrawImage(this->system->ppu.GetPatternTable(0, this->nSelectedPalette), ivec2(Section3Pos.x, Section3Pos.y+148), "PT1");
                    this->DrawImage(this->system->ppu.GetPatternTable(1, this->nSelectedPalette), ivec2(Section3Pos.x+130, Section3Pos.y+148), "PT2");
                    break;

                case Debug_Sprites:
                    for(int i = 0; i < 16; i++){
                        std::string s= hex(i, 2) + ": (" + std::to_string(system->ppu.pOAM[i * 4 + 3])
                            + ", " + std::to_string(system->ppu.pOAM[i * 3 + 0]) + ") "
                            + "ID: " + hex(system->ppu.pOAM[i*4 + 1], 2) +
                            + " AT: " + hex(system->ppu.pOAM[i * 4 + 2], 2);
                        DrawString(ivec2(Section3Pos.x, Section3Pos.y + i * 10), s, vec3(1), ("Sprites" + to_string(i)));
                    }
                    break;

                case Debug_Audio:
                    this->Audios->Configure(ivec2(250, 120), vec3(0));
                    //if (CurrentFrame % static_cast<int>(1 / Time.deltaTime) == 0){
                        this->audioV[0].pop_front();
                        this->audioV[0].push_back(this->system->apu.pulse1_visual);
                        this->audioV[1].pop_front();
                        this->audioV[1].push_back(this->system->apu.pulse2_visual);
                        this->audioV[2].pop_front();
                        this->audioV[2].push_back(this->system->apu.noise_visual);
                        this->audioV[3].pop_front();
                        this->audioV[3].push_back(this->system->apu.triangle_visual);
                    //}
                    DrawAudio(1, Section3Pos.x, 0);
                    DrawAudio(2, Section3Pos.x, 0);
                    DrawAudio(3, Section3Pos.x, 0);
                    DrawAudio(4, Section3Pos.x, 0);

                    this->DrawImage(*this->Audios, ivec2(Section3Pos.x, Section3Pos.y + 120), "Audio" + to_string(0));
                    break;

                case Debug_None:
                    break;
                }
            }

            void DrawAudio(int Channel, int x, int y){
                int i = 0;
                for (auto s : this->audioV[Channel-1]){
                    //cout << "I: " << i << "X: " << x + i << " Y: " << y + (s >> (Channel-1 == 2 ? 5 : 4)) << endl;

                    vec3 Color = vec3(1);

                    switch (Channel){
                        default:
                            this->Audios->SetPixel(ivec2(i, y + (s >> (Channel-1 == 2 ? 5 : 4))), vec4(Color, 1));
                            break;
                        case 2:
                            Color = vec3(1, 0, 0);
                            this->Audios->SetPixel(ivec2(i, y + (s >> (Channel-1 == 2 ? 5 : 4))), vec4(Color, 1));
                            break;
                        case 3:
                            Color = vec3(0, 1, 0);
                            this->Audios->SetPixel(ivec2(i + 130, y + (s >> (Channel-1 == 2 ? 5 : 4))), vec4(Color, 1));
                            break;
                        case 4:
                            Color = vec3(0, 0.5f, 1);
                            this->Audios->SetPixel(ivec2(i + 130, y + (s >> (Channel-1 == 2 ? 5 : 4))), vec4(Color, 1));
                            break;
                    }

                    i++;
                }
            }

            void DrawRam(int x, int y, uint16_t nAddr, int nRows, int nColumns)
            {
                int nRamX = x, nRamY = y;
                for (int row = 0; row < nRows; row++)
                {
                    std::string sOffset = "$" + hex(nAddr, 4) + ":";
                    for (int col = 0; col < nColumns; col++)
                    {
                        sOffset += " " + hex(system->cpuRead(nAddr, true), 2);
                        nAddr += 1;
                    }
                    DrawString(vec2(nRamX, nRamY), sOffset, vec3(1), ("RAM." + to_string(row) + ".$" + hex(nAddr, 4)).c_str());
                    nRamY += 10;
                }
            }

            void DrawCpu(int x, int y)
            {
                std::string status = "STATUS: ";
                DrawString(vec2(x , y) , "STATUS:", vec3(1), "Status");
                DrawString(vec2(x  + 64, y), "N", system->cpu.status & CPU6502::N ? vec3(0, 1, 0) : vec3(1, 0, 0), "Status.N");
                DrawString(vec2(x  + 80, y) , "V", system->cpu.status & CPU6502::V ? vec3(0, 1, 0) : vec3(1, 0, 0), "Status.V");
                DrawString(vec2(x  + 96, y) , "-", system->cpu.status & CPU6502::U ? vec3(0, 1, 0) : vec3(1, 0, 0), "Status.-");
                DrawString(vec2(x  + 112, y) , "B", system->cpu.status & CPU6502::B ? vec3(0, 1, 0) : vec3(1, 0, 0), "Status.B");
                DrawString(vec2(x  + 128, y) , "D", system->cpu.status & CPU6502::D ? vec3(0, 1, 0) : vec3(1, 0, 0), "Status.D");
                DrawString(vec2(x  + 144, y) , "I", system->cpu.status & CPU6502::I ? vec3(0, 1, 0) : vec3(1, 0, 0), "Status.I");
                DrawString(vec2(x  + 160, y) , "Z", system->cpu.status & CPU6502::Z ? vec3(0, 1, 0) : vec3(1, 0, 0), "Status.Z");
                DrawString(vec2(x  + 178, y) , "C", system->cpu.status & CPU6502::C ? vec3(0, 1, 0) : vec3(1, 0, 0), "Status.C");
                DrawString(vec2(x , y + 10), "PC: $" + hex(system->cpu.pc, 4), vec3(1), "PC");
                DrawString(vec2(x , y + 20), "A: $" +  hex(system->cpu.a, 2) + "  [" + std::to_string(system->cpu.a) + "]", vec3(1), "Accumulator");
                DrawString(vec2(x , y + 30), "X: $" +  hex(system->cpu.x, 2) + "  [" + std::to_string(system->cpu.x) + "]", vec3(1), "X");
                DrawString(vec2(x , y + 40), "Y: $" +  hex(system->cpu.y, 2) + "  [" + std::to_string(system->cpu.y) + "]", vec3(1), "Y");
                DrawString(vec2(x , y + 50), "Stack P: $" + hex(system->cpu.stkp, 4), vec3(1), "Stack");

                DrawString(vec2(x , y + 60), "CPU Clock Speed: " + to_string(ClockSpeed/3), vec3(1), "CPUClockSpeed");
                DrawString(vec2(x , y + 70), "PPU Clock Speed: " + to_string(ClockSpeed), vec3(1), "PPUClockSpeed");
                DrawString(vec2(x , y + 80), "APU Clock Speed: " + to_string(ClockSpeed), vec3(1), "APUClockSpeed");

                DrawString(vec2(x , y + 90), "Clock: " + to_string(system->SystemClockCount), vec3(1), "ClockCount");
            }

            void DrawCode(int x, int y, int nLines)
            {
                auto it_a = mapAsm.find(system->cpu.pc);
                int nLineY = (nLines >> 1) * 10 + y;
                if (it_a != mapAsm.end())
                {
                    DrawString(vec2(x, nLineY), (*it_a).second, vec3(0, 1, 1), "it_a1");
                    while (nLineY < (nLines * 10) + y)
                    {
                        nLineY += 10;
                        if (++it_a != mapAsm.end())
                        {
                            DrawString(vec2(x, nLineY), (*it_a).second, vec3(1),("CODE." + to_string(nLineY) + "." + "it_a2").c_str());
                        }
                    }
                }

                it_a = mapAsm.find(system->cpu.pc);
                nLineY = (nLines >> 1) * 10 + y;
                if (it_a != mapAsm.end())
                {
                    while (nLineY > y)
                    {
                        nLineY -= 10;
                        if (--it_a != mapAsm.end())
                        {
                            DrawString(vec2(x, nLineY), (*it_a).second, vec3(1), ("CODE." + to_string(nLineY) + "." + "it_3").c_str());
                        }
                    }
                }
            }

            void DrawRoms(int x, int y){
                int yAdd = 0;

                for (std::filesystem::path p : GetRoms()){
                    DrawString(ivec2(x, y + yAdd), p.filename().string(), vec3(1), ("ROM::NES:" + to_string(yAdd)));
                    yAdd+=10;
                }
            }

        private:

            static float SoundOut(int nChannel, float fGlobalTime, float fTimeStep){
                if (nChannel == 0){
                    while (!emulatorPointer->system->clock()) {};
                    if(*PlayAudio)
                        return static_cast<float>(emulatorPointer->system->dAudioSample);
                    else
                        return 0.0f;
                }
                else
                    return 0.0f;
            }

        public:
            void UpdateController(int id = 0){
                // Handle input for controller in port #1
                this->system->controller[id] = 0x00;
                if(!this->Controllers[id].ControlerNotKeyboard){
                    this->system->controller[id] |= this->game->Input.Keyboard.KeyPressed(this->Controllers[id].A) ? 0x80 : 0x00; // A Button
                    this->system->controller[id] |= this->game->Input.Keyboard.KeyPressed(this->Controllers[id].B) ? 0x40 : 0x00; // B Button
                    this->system->controller[id] |= this->game->Input.Keyboard.KeyPressed(this->Controllers[id].Select) ? 0x20 : 0x00; // Select
                    this->system->controller[id] |= this->game->Input.Keyboard.KeyPressed(this->Controllers[id].Start) ? 0x10 : 0x00; // Start
                    this->system->controller[id] |= this->game->Input.Keyboard.KeyPressed(this->Controllers[id].Up) ? 0x08 : 0x00;
                    this->system->controller[id] |= this->game->Input.Keyboard.KeyPressed(this->Controllers[id].Down) ? 0x04 : 0x00;
                    this->system->controller[id] |= this->game->Input.Keyboard.KeyPressed(this->Controllers[id].Left) ? 0x02 : 0x00;
                    this->system->controller[id] |= this->game->Input.Keyboard.KeyPressed(this->Controllers[id].Right) ? 0x01 : 0x00;
                }
            }

            void Update(){
                this->CurrentFrame++;

                UpdateController(0);

                if(this->ToggleExtraController)
                    UpdateController(1);               

                if(this->game->Input.Keyboard.KeyPressed(Key_R) && !this->Button_Pressed){
                    this->system->reset();
                    this->Button_Pressed = true;
                    this->lastKey = Key_R;
                }
                else if(!this->game->Input.Keyboard.KeyPressed(Key_R) && this->Button_Pressed && this->lastKey == Key_R){
                    this->Button_Pressed = false;
                    this->lastKey = Key_0;
                }

                if(this->DebugMode)
                    if(this->game->Input.Keyboard.KeyPressed(Key_P) && !this->Button_Pressed){
                        (++this->nSelectedPalette) &= 0x07;
                        this->Button_Pressed = true;
                        this->lastKey = Key_P;
                    }
                    else if(!this->game->Input.Keyboard.KeyPressed(Key_P) && this->Button_Pressed && this->lastKey == Key_P){
                        this->Button_Pressed = false;
                        this->lastKey = Key_0;
                    }
                
                if(this->RomHotSwap){
                    if(this->game->Input.Keyboard.KeyPressed(Key_N) && !this->Button_Pressed){
                        this->CurrentRom++;
                        if(this->CurrentRom >= this->GetRoms().size()){
                            this->CurrentRom = 0;
                        }

                        this->cart = std::make_shared<Cartridge>(this->GetRoms()[this->CurrentRom].string());

                        this->system->insertCartridge(cart);

                        this->system->reset();

                        this->Button_Pressed = true;
                        this->lastKey = Key_N;
                    }
                    else if(!this->game->Input.Keyboard.KeyPressed(Key_N) && this->Button_Pressed && this->lastKey == Key_N && this->cartChangeInterval % 10 == 0){
                        this->Button_Pressed = false;
                        this->lastKey = Key_0;
                    }
                }

                if(this->DebugMode){
                    if(this->game->Input.Keyboard.KeyPressed(Key_PAGE_UP) && !this->Button_Pressed && this->current_scale <= 2){
                        this->game->window.Size *= 2;
                        this->current_scale *= 2;
                        this->lastKey = Key_PAGE_UP;
                        this->Button_Pressed = true;
                    }
                    else if(!game->Input.Keyboard.KeyPressed(Key_PAGE_UP) && this->Button_Pressed && this->lastKey == Key_PAGE_UP){
                        this->Button_Pressed = false;
                        this->lastKey = Key_0;
                    }

                    if(game->Input.Keyboard.KeyPressed(Key_PAGE_DOWN) && !this->Button_Pressed && this->current_scale >= 2){
                        this->game->window.Size /= 2;
                        this->current_scale /= 2;
                        this->lastKey = Key_PAGE_DOWN;
                        this->Button_Pressed = true;
                    }
                    else if(!game->Input.Keyboard.KeyPressed(Key_PAGE_DOWN) && this->Button_Pressed && this->lastKey == Key_PAGE_DOWN){
                        this->Button_Pressed = false;
                        this->lastKey = Key_0;
                    }
                }
                
                if(this->DebugMode)
                    if(this->CurrentFrame % 1/Time.deltaTime == 0){
                        this->ClockSpeed = this->system->ClockSpeedCounter;
                        this->system->ClockSpeedCounter = 0;
                    }
                this->cartChangeInterval++;
                
                if(this->DebugMode)
                    this->DrawDebug();
                    this->DrawImage(this->system->ppu.GetScreen(), ivec2(0, 475), "MainView");
            }

            void Render(){
                if(this->DebugMode)
                    this->game->Render();
                else
		            this->game->Render(&this->system->ppu.GetScreen(), 4);
            }
        };

        NESEmulator* NESEmulator::emulatorPointer = nullptr;
        bool* NESEmulator::PlayAudio = nullptr;
    }
}