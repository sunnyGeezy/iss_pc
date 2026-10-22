#include "../../src/iss_karma.h"
#include "imgui.h"
#include "tlg_karma_backup.h"
#include <atomic>
#include <chrono>
#include <cstring>
#include <stdexcept>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

extern "C" const char *runtime_file_user_dir(void);
namespace {
std::atomic<bool> backing_up{false};
std::mutex message_lock;
std::string message;
void Message(std::string text){std::lock_guard guard(message_lock);message=std::move(text);}
void Prepare(int mode){
    if(backing_up.exchange(true))return;
    Message("Backing up saves...");
    // Copy the root before starting the worker; no game memory is accessed there.
    const std::filesystem::path root=runtime_file_user_dir();
    std::thread([root,mode]{
        namespace fs=std::filesystem;
        try{
            const auto source=root/"savedata"/"1"/"CUSA00223";
            const auto stamp=std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
            const auto destination=root/"karma-backups"/(std::to_string(stamp)+(mode==1?"-true-hero":"-infamous"));
            TlgKarmaBackup(source,destination);
            IssKarmaStatus status{};runtime_karma_status(&status);
            if(!status.supported || !status.ready)throw std::runtime_error("Return to gameplay and try again");
            runtime_karma_request(mode);
            Message("Backup verified. Switch queued for gameplay.");
        }catch(const std::exception& e){Message(std::string("Switch cancelled: ")+e.what());}
        backing_up.store(false);
    }).detach();
}
}
void TlgKarmaMenu(){
    IssKarmaStatus s{};runtime_karma_status(&s);
    ImGui::TextUnformatted("Karma Switcher");
    ImGui::Separator();
    ImGui::TextWrapped("Choose one karma branch at Level 5. Upgrades for powers you have earned are unlocked for that branch. Switching takes effect when gameplay resumes.");
    if(!s.supported){ImGui::TextDisabled("Unavailable: requires the verified CUSA00223 update 1.07 build.");return;}
    ImGui::Text("Current: %s | Level %d | %.1f",s.side==1?"Good":"Evil",s.rank,s.score);
    ImGui::Text("Selected mode: %s",s.mode==1?"True Hero":s.mode==2?"Infamous":"Unchanged");
    ImGui::BeginDisabled(!s.ready || s.pending || backing_up.load());
    if(ImGui::Button("True Hero"))Prepare(1);
    ImGui::SameLine();
    if(ImGui::Button("Infamous"))Prepare(2);
    ImGui::EndDisabled();
    if(!s.ready)ImGui::TextDisabled("Load a save and enter gameplay first.");
    if(s.error)ImGui::TextWrapped("Switch could not complete (%d). Your backup remains available.",s.error);
    {std::lock_guard guard(message_lock);ImGui::TextWrapped("%s",message.c_str());}
    ImGui::Spacing();
    ImGui::TextWrapped("A verified backup is made before each switch, in karma-backups inside your data folder. Story progress and collectibles are not edited. Normal autosaves may record the selected karma and granted upgrades.");
    ImGui::TextDisabled("The selected mode keeps karma at maximum for this session.");
}
