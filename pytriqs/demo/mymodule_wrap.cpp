#include <boost/python.hpp>
#include <iostream>

using namespace boost::python;

class myClass
{
  public:
    myClass(object ob){ U = extract<int>(ob.attr("U")); }
    myClass (int u): U(u) { }

    void solve() { std::cout << "Je suis dans le C++\n" << "U = " << U << std::endl; }
    int U;
};


int increment(int x) { return x+1;}

void read_dict(boost::python::dict d) { 

 int i = extract<int>(d['i']);
 std::string s = extract<std::string>(d['s']);

 std::cout << "i = " << i<< std::endl ;
 std::cout << "s = " << s<< std::endl ;
 
 // a more complex example....
 myClass c = extract<myClass>(d['C']);
  std::cout  << " myclass .U = " << c.U<< std::endl;

}


boost::python::dict modify_dict(boost::python::dict d) { 

  d['i'] *=2;
 return d;
}


myClass make_myclass(int i) { return myClass(i);} 

BOOST_PYTHON_MODULE(mymodule)
{
    class_<myClass>("myClass", init<object>())
      .def("solvecpp",&myClass::solve)
      .add_property("U",&myClass::U)
    ;

    def ("inc", increment,"This is my nice inc function ");
    def ("read_dict", read_dict,"Reading a simple dict ....");
    def ("modify_dict", modify_dict,"");
    def ("make_myclass", make_myclass,"");

};
