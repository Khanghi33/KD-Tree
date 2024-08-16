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
	float coord[k];
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
	else {
		if (root->right != NULL) INSERT(root->right, newcity, depth + 1);
		else root->right = CREATE_NODE(newcity);
	}
}
KDNODE* GET_DATA(string filename) {
	KDNODE* root = NULL;
	ifstream fin(filename, ios::in);
	if (!fin) cout << "Cannot open file!!\n";
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
	}
	fin.close();
	return root;
}
void check_range(vector<CITY>& result, KDNODE* root, float top_right[k], float bot_left[k], int depth) {
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
vector<CITY> RANGE_SEARCH(KDNODE* root, float top_right[k], float bot_left[k]) {
	vector<CITY> result;
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
float Distance(float point1[k], float point2[k]) {
	return sqrt((point1[0] - point2[0]) * (point1[0] - point2[0])
		   + (point1[1] - point2[1]) * (point1[1] - point2[1]));
}
const float R = 6371.0;
const float M_PI = 3.14156;
float HAVERSINE_DISTANCE(float point1[k], float point2[k]) {
	float dLat = (point2[0] - point1[0]) * M_PI / 180.0;
	float dLon = (point2[1] - point1[1]) * M_PI / 180.0;
	float lat1 = point1[0] * M_PI / 180.0;
	float lat2 = point2[0] * M_PI / 180.0;
	float a = sin(dLat / 2) * sin(dLat / 2) +
		cos(lat1) * cos(lat2) *
		sin(dLon / 2) * sin(dLon / 2);
	float c = 2 * atan2(sqrt(a), sqrt(1 - a));
	return R * c;
}
void check_nearest(KDNODE* root, float point[k], CITY& nearest_city, float min_dis, int depth) {
	int axis = depth % 2;
	if (root == NULL) return;
	bool left = true, right = true;
	if (min_dis >= HAVERSINE_DISTANCE(root->city.coord, point)) {
		nearest_city = root->city;
		min_dis = HAVERSINE_DISTANCE(root->city.coord, point);
	}
	else {
		if (root->city.coord[axis] > point[axis])
			right = false;
		if (root->city.coord[axis] < point[axis])
			left = false;
	}
	if (left) check_nearest(root->left, point, nearest_city, min_dis, depth + 1);
	if (right) check_nearest(root->right, point, nearest_city, min_dis, depth + 1);
}
CITY NEAREST_SEARCH(KDNODE* root, float point[k]) {
	CITY nearest_city;
	check_nearest(root, point, nearest_city, INT_MAX, 0);
	return nearest_city;
}
void SERIALIZE(string filename, KDNODE* root) {
	ofstream fout(filename, ios::out | ios::binary);
	if (!fout) cout << "Cannot open file!!";
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
	}
	fout.close();
}
KDNODE* DESERIALIZE(string filename) {
	KDNODE* root = NULL;
	ifstream fin(filename, ios::in | ios::binary);
	if (!fin) cout << "Cannot open file!!";
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
float main() {
	KDNODE* root = GET_DATA("worldcities.csv");
	/*float points[10][k] = { {3, 6}, {17, 15}, {13, 15}, {6, 12},
					   {9, 1}, {2, 7}, {10, 19} };*/
	//PRINT(root); cout << endl;
	float bot_left[k] = { 33, 124 };
	float top_right[k] = { 43, 132 };
	cout << "CITY IN AREA " << "(" << bot_left[0] << ";" << bot_left[1] << ") -> ("
		<< top_right[0] << ";" << top_right[1] << ")\n";
	vector<CITY> result = RANGE_SEARCH(root, top_right, bot_left);
	for (int i = 0; i < result.size(); i++) {
		cout << result[i].name;
		cout << " (";
		for (int j = 0; j < k; j++) {
			cout << result[i].coord[j];
			if (j != 1)
				cout << ";";
		}
		cout << ")\n";
	}
	cout << endl;
	float coord[k] = {37.5, 127 };
	CITY nearest_city = NEAREST_SEARCH(root, coord);
	cout << "Nearest city with " << "(" << coord[0] << ";" << coord[1] << ") : "
		 << nearest_city.name << " (" << nearest_city.coord[0] << ";" << nearest_city.coord[1] << ")" << endl;
	SERIALIZE("worldcitites.bin", root);
	KDNODE* newroot = DESERIALIZE("worldcitites.bin");
	//PRINT(newroot);
	DeleteRoot(root);
	DeleteRoot(newroot);
	return 0;
}
