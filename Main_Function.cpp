#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <cmath>
#include <stack>
using namespace std;
const int k = 2;
struct CITY {
	string name;
	//Save latitude and longtitude
	double coord[k];
};
struct KDNODE {
	CITY city;
	KDNODE* left;
	KDNODE* right;
};
KDNODE* CREATE_NODE(CITY INFO) {
	KDNODE* newnode = new KDNODE();
	newnode->city = INFO;
	newnode->left = NULL;
	newnode->right = NULL;
	return newnode;
}
void INSERT(KDNODE*& root, CITY newcity, int depth) {
	//Find which value need to compare in k-dimension (x1, x2,...,xk)
	int xy = depth % k;
	if (root == NULL) root = CREATE_NODE(newcity);
	else if (newcity.coord[xy] < root->city.coord[xy]) {
		if (root->left != NULL) INSERT(root->left, newcity, depth + 1);
		else root->left = CREATE_NODE(newcity);
	}
	//Include case point[xy] > root->city.coord[xy] and point[xy] = root->city.coord[xy]
	else  {
		if (newcity.coord[0] == root->city.coord[0] &&
			newcity.coord[1] == root->city.coord[1] &&
			newcity.name == root->city.name) {
			cout << "City: " << newcity.name << " " << "(" << newcity.coord[0] << ";"
				<< newcity.coord[1] << ")" << " has been existed!! " << endl;
		}
		else {
			if (root->right != NULL) INSERT(root->right, newcity, depth + 1);
			else root->right = CREATE_NODE(newcity);
		}
	}
}
KDNODE* GET_DATA(string filename) {
	KDNODE* root = NULL;
	ifstream fin(filename, ios::in);
	if (!fin)
	{
		cout << "Cannot open file!!\n";
		return NULL;
	}
	else {
		string trash, line;
		getline(fin, trash);
		while (getline(fin, line)) {
			CITY info;
			stringstream ss(line);
			string name, lat, lng;
			getline(ss, trash, ',');
			getline(ss, name, ',');
			getline(ss, lat, ',');
			getline(ss, lng, ',');
			getline(ss, trash);
			info.name = name.substr(1, name.length() - 2);
			//Save latitude
			stringstream(lat.substr(1, lat.length() - 2)) >> info.coord[0];
			//Save longtitude
			stringstream(lng.substr(1, lng.length() - 2)) >> info.coord[1];
			INSERT(root, info, 0);
		}
		cout << "Read successfully" << endl;
	}
	fin.close();
	return root;
}
void check_range(vector<CITY>& result, KDNODE* root, double top_right[k], double bot_left[k], int depth) {
	int axis = depth % 2;
	if (root == NULL) return;
	//Check if it in the rectangle add it to result
	if (root->city.coord[0] > bot_left[0] && root->city.coord[0] < top_right[0]
		&& root->city.coord[1] > bot_left[1] && root->city.coord[1] < top_right[1]) {
		CITY inarea_city = root->city;
		result.push_back(inarea_city);
	}
	bool right = true, left = true;
	//Check if it get over of the rectangle, it wouldn't call recur
	if (bot_left[axis] > root->city.coord[axis])
		left = false;
	if (top_right[axis] < root->city.coord[axis])
		right = false;
	if (left) check_range(result, root->left, top_right, bot_left, depth + 1);
	if (right) check_range(result, root->right, top_right, bot_left, depth + 1);
}
//Coordinate of bot_left -> top_right create a Rectangle area
//This function will find all city in that area
vector<CITY> RANGE_SEARCH(KDNODE* root, double top_right[k], double bot_left[k]) {
	vector<CITY> result;
	if (root == NULL) {
		cout << "Data isn't existed!!\n";
		return result;
	}
	check_range(result, root, top_right, bot_left, 0);
	return result;
}
void PRINT(KDNODE* root) {
	if (root == NULL) return;
	PRINT(root->left);
	cout << root->city.name << " ";
	cout << "(";
	for (int i = 0; i < k; i++) {
		cout << root->city.coord[i];
		if (i != k - 1)
			cout << ";";
	}
	cout << ")\n";
	PRINT(root->right);
}
double Distance(double point1[k], double point2[k]) {
	return sqrt((point1[0] - point2[0]) * (point1[0] - point2[0])
		   + (point1[1] - point2[1]) * (point1[1] - point2[1]));
}
const double R = 6371.0;
const double M_PI = 3.14156;
double HAVERSINE_DISTANCE(double point1[k], double point2[k]) {
	double dLat = (point2[0] - point1[0]) * M_PI / 180.0;
	double dLon = (point2[1] - point1[1]) * M_PI / 180.0;
	double lat1 = point1[0] * M_PI / 180.0;
	double lat2 = point2[0] * M_PI / 180.0;
	double a = sin(dLat / 2) * sin(dLat / 2) +
		cos(lat1) * cos(lat2) *
		sin(dLon / 2) * sin(dLon / 2);
	double c = 2 * atan2(sqrt(a), sqrt(1 - a));
	return R * c;
}
void check_nearest(KDNODE* root, double point[k], CITY& nearest_city, double& min_dis, int depth) {
	//Find which axis should be checked with a depth
	int axis = depth % 2;
	if (root == NULL) return;
	bool left = true, right = true;
	//If the current node has the distance smaller than the min distance
	//-> Update min distance and nearest city
	if (min_dis >= HAVERSINE_DISTANCE(root->city.coord, point)) {
		nearest_city = root->city;
		min_dis = HAVERSINE_DISTANCE(root->city.coord, point);
	}
	//Check if the continue children should be checked 
	else {
		if (root->city.coord[axis] > point[axis])
			right = false;
		else if (root->city.coord[axis] < point[axis])
			left = false;
	}
	if (left) check_nearest(root->left, point, nearest_city, min_dis, depth + 1);
	if (right) check_nearest(root->right, point, nearest_city, min_dis, depth + 1);
	//Finally, check otherside which didn't call recursive
	//if it has the dis < min_dis -> continue check it
	double diff = fabs(root->city.coord[axis] - point[axis]);
	double diff_rad = diff * M_PI / 180.0;
	if (2 * R * asin(sin(diff_rad / 2)) < min_dis) {
		if (!left) check_nearest(root->left, point, nearest_city, min_dis, depth + 1);
		else check_nearest(root->right, point, nearest_city, min_dis, depth + 1);
	}
}
CITY NEAREST_SEARCH(KDNODE* root, double point[k]) {
	CITY nearest_city;
	if (root == NULL) {
		cout << "Data isn't existed!!\n"; return nearest_city;
	}
	double min_dis = INT_MAX;
	check_nearest(root, point, nearest_city, min_dis, 0);
	//cout << "Distance: " << min_dis << endl;
	return nearest_city;
}
void SERIALIZE(string filename, KDNODE* root) {
	if (root == NULL) {
		cout << "No data to serialize!!\n";
		return;
	}
	ofstream fout(filename, ios::out | ios::binary);
	if (!fout) cout << "Cannot open file!!\n";
	else {
		stack<KDNODE*> s; s.push(root);
		while (!s.empty()) {
			KDNODE* node = s.top(); s.pop();
			if (node->right) s.push(node->right);
			if (node->left) s.push(node->left);

			int name_length = node->city.name.length() + 1;
			fout.write((char*)&name_length, sizeof(int));
			fout.write(node->city.name.data(), name_length);
			fout.write((char*)&node->city.coord, sizeof(node->city.coord));
		}
		cout << "Serialize succesfully!!\n";
	}
	fout.close();
}
KDNODE* DESERIALIZE(string filename) {
	KDNODE* root = NULL;
	ifstream fin(filename, ios::in | ios::binary);
	if (!fin) cout << "Cannot open file!!\n";
	else {
		while (!fin.eof()) {
			CITY newcity;
			int name_length;
			fin.read((char*)&name_length, sizeof(int));
			char* name = new char[name_length];
			fin.read(name, name_length);
			newcity.name.resize(name_length);
			newcity.name.assign(name);
			fin.read((char*)&newcity.coord, sizeof(newcity.coord));
			INSERT(root, newcity, 0);
		}
		cout << "Deserialize successfully!!\n";
	}
	fin.close();
	return root;
}
void DeleteRoot(KDNODE* &root) {
	if (root == NULL) return;
	DeleteRoot(root->left);
	DeleteRoot(root->right);
	delete root;
	root = NULL;
}
void PrintCity(vector<CITY>& city)
{
	for (auto it : city)
	{
		cout << it.name << ", " << it.coord[0] << ", " << it.coord[1] << endl;
	}
}
void PrintCityListFile(vector<CITY>& city, string filename)
{
	fstream fout(filename, ios::out);
	if (!fout) cout << "File not found" << endl;
	
	else
	{
		fout << "City name,latitude,longtitude" << endl;
		for (auto it : city)
		{
			fout << it.name << "," << it.coord[0] << "," << it.coord[1] << endl;
		}
	}
	cout << "Write to file successfully!" << endl;
	fout.close();
}
void PrintCityFile(CITY& city, string filename)
{
	fstream fout(filename, ios::out);
	if (!fout) cout << "File not found" << endl;

	else
	{
		fout << "City name,latitude,longtitude" << endl;
		fout << city.name << "," << city.coord[0] << "," << city.coord[1];
	}
	cout << "Write to file successfully!" << endl;
	fout.close();
}
int main() {
	cout << "Choose option(Enter valid number): " << endl;
	cout << "1. Load CSV file" << endl;
	cout << "2. Insert a new city from command line " << endl;
	cout << "3. Insert multiple cities from a csv file" << endl;
	cout << "4. Conduct a nearest neighbor search by providing latitude and longtitude" << endl;
	cout << "5. Query all cities within a specified rectangular region defined by its geographical boundaries" << endl;
	cout << "6. Serialize to storing data to binary file " << endl;
	cout << "7. Deserialize data from binary file and rebuild " << endl;
	cout << "8. Exit" << endl;
	cout << "--------------------------------------------------------------------------------------------------" << endl;
	KDNODE* root = NULL;
	while (true)
	{
		
		cout << "Enter option: ";
		int op; cin >> op;
		cin.ignore(256, '\n');
		if (op == 1)
		{
			string filename;
			cout << "Enter filename: ";
			getline(cin, filename);
			root = GET_DATA(filename);
			cout << "--------------------------------------------------------------------------------------------------" << endl;
		}
		else if (op == 2)
		{
			CITY city;
			cout << "Input city name: ";
			getline(cin, city.name);
			cout << "Input city's latitude: ";
			cin >> city.coord[0];
			cin.ignore(256, '\n');
			cout << "Input city's longtitude: ";
			cin >> city.coord[1];
			cin.ignore(256, '\n');
			INSERT(root, city, 0);
			cout << "--------------------------------------------------------------------------------------------------" << endl;
		}
		else if (op == 3)
		{
			cout << "Input file name: ";
			string filename;
			getline(cin, filename);
			ifstream fin(filename, ios::in);
			if (!fin)
			{
				cout << "Cannot open file!!\n";
			}
			else {
				string trash, line;
				getline(fin, trash);
				while (getline(fin, line)) {
					CITY info;
					stringstream ss(line);
					string name, lat, lng;
					getline(ss, trash, ',');
					getline(ss, name, ',');
					getline(ss, lat, ',');
					getline(ss, lng, ',');
					getline(ss, trash);
					info.name = name.substr(1, name.length() - 2);
					//Save latitude
					stringstream(lat.substr(1, lat.length() - 2)) >> info.coord[0];
					//Save longtitude
					stringstream(lng.substr(1, lng.length() - 2)) >> info.coord[1];
					INSERT(root, info, 0);
				}
				cout << "Read successfully" << endl;
			}
			fin.close();
			cout << "--------------------------------------------------------------------------------------------------" << endl;
		}
		else if (op == 4) //Conduct a nearest neighbor search by providing latitude and longtitude
		{
			double coord[k];
			cout << "Input city's latitude: "; cin >> coord[0];
			cin.ignore(256, '\n');
			cout << "Input city's longtitude: "; cin >> coord[1];
			cin.ignore(256, '\n');
			CITY nearest_city = NEAREST_SEARCH(root, coord);
			int choice;
			cout << "Enter 1 for CLI output | or 2 for file output | or 3 for both: "; cin >> choice;
			cin.ignore(256, '\n');
			if (choice == 1) // Command line only
			{
				cout << "City name: " << nearest_city.name << endl;
				cout << "City's latitude, longtitude: " << nearest_city.coord[0] << ", " << nearest_city.coord[1] << endl;
			}
			else if (choice == 2) // File only
			{
				string fileout;
				cout << "Input output file: ";
				getline(cin, fileout);
				PrintCityFile(nearest_city, fileout);
			}
			else if (choice == 3) // Both command line and file
			{
				// Command line output
				cout << "City name: " << nearest_city.name << endl;
				cout << "City's latitude, longtitude: " << nearest_city.coord[0] << ", " << nearest_city.coord[1] << endl;
				// File output
				string fileout;
				cout << "Input output file: ";
				getline(cin, fileout);
				PrintCityFile(nearest_city, fileout);
			}
			cout << "--------------------------------------------------------------------------------------------------" << endl;
		}
		else if (op == 5) // Query all cities within a specified rectangular region defined by its geographical boundariesQuery all cities within a specified rectangular region defined by its geographical boundaries
		{
			// Input bottom left
			double bottom_left[k];
			cout << "Input bottom left's latitude of rectangle: "; cin >> bottom_left[0];
			cin.ignore(256, '\n');
			cout << "Input bottom left's longtitude of rectangle: "; cin >> bottom_left[1];
			cin.ignore(256, '\n');
			// Input top right
			double top_right[k];
			cout << "Input top_right's latitude of rectangle: "; cin >> top_right[0];
			cin.ignore(256, '\n');
			cout << "Input top right's longtitude of rectangle: "; cin >> top_right[1];
			cin.ignore(256, '\n');

			vector<CITY> result = RANGE_SEARCH(root, top_right, bottom_left); // Range search
			int choice;
			cout << "Enter 1 for CLI output | or 2 for file output | or 3 for both: "; cin >> choice;
			cin.ignore(256, '\n');
			if (choice == 1) // Command line only
			{
				cout << "City name, latitude, longtitude" << endl;
				PrintCity(result);
			}
			else if (choice == 2) // File only
			{
				string fileout;
				cout << "Input output file: ";
				getline(cin, fileout);
				fstream fout(fileout, ios::out);
				PrintCityListFile(result, fileout);
			}
			else if (choice == 3) // Both command line and file
			{
				// Command line output
				cout << "City name, latitude, longtitude" << endl;
				PrintCity(result);
				// File output
				string fileout;
				cout << "Input output file: ";
				getline(cin, fileout);
				fstream fout(fileout, ios::out);
				PrintCityListFile(result, fileout);
			}
			cout << "--------------------------------------------------------------------------------------------------" << endl;
		}
		else if (op == 6) {
			string filename;
			cout << "Input file name: ";
			getline(cin, filename);
			SERIALIZE(filename, root);
		}
		else if (op == 7) {
			DeleteRoot(root);
			root = NULL;
			string filename;
			cout << "Input file name: ";
			getline(cin, filename);
			root = DESERIALIZE(filename);
		}
		else if (op == 8) break;

	}
	DeleteRoot(root);
	return 0;
}
