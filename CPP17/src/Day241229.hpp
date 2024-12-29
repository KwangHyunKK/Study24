#include <unordered_map>
#include <list>
#include <filesystem>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <deque>
#include <fstream>
#include <iostream>

namespace D241229
{
    /// # 1. Filesystem with snapshot

    // Structure of file snapshot
    struct FileSnapshot
    {
        std::string __content;
        std::string __hash;
        std::string __timestamp;

        FileSnapshot(const std::string& content, const std::string& hash, const std::string& timestamp)
            : __content(content), __hash(hash), __timestamp(timestamp) {}

        FileSnapshot(const FileSnapshot& oth) = default;
        FileSnapshot(FileSnapshot&& oth) = default;
    };


    // Helper function to caculate a simple hash
    std::string calculateHash(const std::string& content, const std::filesystem::path& filePath)
    {
        std::hash<std::string> hasher;
        auto lastWriteTime = std::filesystem::last_write_time(filePath);
        auto size = std::filesystem::file_size(filePath);

        std::stringstream ss;
        ss << content << lastWriteTime.time_since_epoch().count() << size;

        return std::to_string(hasher(ss.str()));
    }
    
    // Get current timestamp as string
    std::string getCurrentTimeStamp()
    {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }

    // LCS algorithm for line comparision
    std::deque<std::pair<std::string, std::string>> computeLCS(const std::vector<std::string>& oldLines, const std::vector<std::string>& newLines)
    {
        size_t m = oldLines.size();
        size_t n = newLines.size();

        std::vector<std::vector<int>> lcs(m + 1, std::vector<int>(n+1, 0));

        for(size_t i=1;i<=m;++i)
        {
            for(size_t j = 1;j<=n;++j)
            {
                if(oldLines[i-1] == newLines[j-1])
                {
                    lcs[i][j] = lcs[i-1][j-1] + 1;
                }
                else{
                    lcs[i][j] = std::max(lcs[i-1][j], lcs[i][j-1]);
                }
            }
        }

        std::deque<std::pair<std::string, std::string>> result;

        size_t i = m, j = n;
        while(i > 0 || j > 0)
        {
            if(i > 0 && j > 0 && oldLines[i-1] == newLines[j-1])
            {
                result.push_front({" ", oldLines[i-1]});
                --i; --j;
            }
            else if(j > 0 && (i == 0 || lcs[i][j-1] >= lcs[i-1][j]))
            {
                result.push_front({"+", newLines[j-1]});
                --j;
            }
            else
            {
                result.push_front({"-", oldLines[i-1]});
                --i;
            }
        }
        return result;
    }

    // SnapshotManager
    class SnapshotManager
    {
    private:
        std::unordered_map<std::string, std::list<FileSnapshot>> snapshots;

    public:
        void createSnapshot(const std::string& filePath)
        {
            std::ifstream file(filePath);
            if(!file){
                std:: cerr << "Failed to open file : " << filePath << "\n";
                return;
            }

            std::stringstream buffer;
            buffer << file.rdbuf();
            std::string content = buffer.str();
            std::string hash = calculateHash(content, filePath);
            std::string timestamp = getCurrentTimeStamp();

            snapshots[filePath].emplace_back(content, hash, timestamp);
            std::cout << "Snapshot created for : " << filePath << " at " << timestamp << "\n";
        }

        void compareSnapshot(const std::string& filePath)
        {
            if(snapshots.find(filePath) == snapshots.end() || snapshots[filePath].empty())
            {
                std::cerr << "No snapshot found for : " << filePath << "\n";
                return;
            }

            std::ifstream file(filePath);
            if(!file)
            {
                std::cerr << "Failed to open file : " << filePath << "\n";
                return;
            }

            std::stringstream buffer;
            buffer << file.rdbuf();
            std::string currentContent = buffer.str();

            const auto& lastestSnapshot = snapshots[filePath].back();
            std::vector<std::string> oldLines, newLines;
            std::istringstream oldStream(lastestSnapshot.__content);
            std::istringstream newStream(currentContent);
            std::string line;
            while(std::getline(oldStream, line)) oldLines.push_back(line);
            while(std::getline(newStream, line)) newLines.push_back(line);

            auto result = computeLCS(oldLines, newLines);
            for(const auto& [prefix, line] : result)
            {
                std::cout << prefix << " " << line << "\n";
            }
        }
    };
    
}