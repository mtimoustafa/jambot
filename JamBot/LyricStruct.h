#include <string>

using namespace std;

class LyricStruct{
private:
	string lyrics;
	enum style{ normal, bolded, italic };
public:
	LyricStruct(string, style);
};