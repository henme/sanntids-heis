#include <sstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>

using boost::property_tree::ptree;
using boost::property_tree::read_json;
using boost::property_tree::write_json;

std::string toJSON(int CurrentFloor, int Dir, int FloorArr[4]){
  ptree pt;

  pt.put("CurrentFloor", CurrentFloor);
  pt.put("Dir", Dir);
  pt.put("Floor1", FloorArr[0]);
  pt.put("Floor2", FloorArr[1]);
  pt.put("Floor3", FloorArr[2]);
  pt.put("Floor4", FloorArr[3]);

  std::ostringstream buf; 
  write_json(buf, pt);
  std::string json = buf.str();
  return json;
}

//Eventuelt en fromJSON(ptree json) funksjon om det trengs, kan da fylle inn i state object/array

int main() {
  ptree pt, pt2;
  ptree children;
  ptree floor1, floor2, floor3, floor4;

  floor1.put("", 1);
  floor2.put("", 0);
  floor3.put("", 1);
  floor4.put("", 0);

  children.push_back(std::make_pair("", floor1));
  children.push_back(std::make_pair("", floor2));
  children.push_back(std::make_pair("", floor3));
  children.push_back(std::make_pair("", floor4));

// Virker lettere å ha de som egen int variabel istedet for array, fant ingen fin måte å håndtere array på

  pt.put("CurrentFloor", 1);
  pt.put("Dir", 1);
  pt.add_child("FloorArr", children);
  std::ostringstream buf; 
  write_json(buf, pt);
  std::string json = buf.str();

//Hente data
  std::cout << json << std::endl;
  //konverter til property tree
  std::istringstream is(json);
  read_json (is, pt2);
  //Hent ut data
  int CurrentFloor = pt2.get<int>("CurrentFloor");
  int Dir = pt2.get<int>("Dir");
  
  std::cout << CurrentFloor << std::endl;
  std::cout << Dir << std::endl;

  // Beste måten jeg fant for å hente ut array data...
  BOOST_FOREACH (ptree::value_type &v, pt2.get_child("FloorArr")) {
      std::cout << v.second.data() << std::endl;
  }
  return 0;
}
