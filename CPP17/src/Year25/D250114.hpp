#include <iostream>
#include <vector>
#include <string>

namespace Y25M1
{
    using namespace std;
    class RabinKarp1
    {
    public:
        static int findString(const string& ws, const string& ps)
        {
            int wsSize = ws.size(), psSize = ps.size();
            int wsHash = 0, psHash = 0, power = 1;

            for(int i=0;i<=wsSize - psSize;++i)
            {
                if(i==0)
                {
                    for(int j=0;j<psSize;++j)
                    {
                        wsHash += ws[psSize -1 -j] * power;
                        psHash += ps[psSize-1 - j] * power;
                        if(j < psSize -1) power *= 3;
                    }
                }
                else{
                    wsHash = 3 * (wsHash - ws[i-1] * power) + ws[psSize -1 + i];
                }

                if(wsHash == psHash)
                {
                    bool isFind = true;
                    for(int j=0;j<psSize;++j)
                    {
                        if(ws[i + j] != ps[j])
                        {
                            isFind = false;
                            break;
                        }
                    }

                    if(isFind)
                    {
                        std::cout << wsHash << " " << psHash << "\n";
                        return i + 1;
                    }
                    else
                    {
                        std::cout << "Not found\n";
                        return -1;
                    }
                }
            }
            return -1;
        }
    };
}