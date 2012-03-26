#include <triqs/utility/proto/algebra.hpp>
#include <boost/type_traits/is_complex.hpp> 
#include <vector>
#include <iostream>

namespace mpl = boost::mpl; 
#define TEST(X) std::cout << BOOST_PP_STRINGIZE((X)) << " ---> "<< (X) <<std::endl;

// first define the algebra element.
struct my_vector { 
 std::vector<double> v;
 my_vector():v(3){}
 double operator[](size_t i) const { return v[i];}
 double & operator[](size_t i) { return v[i];}
  size_t size() const {return v.size();}
};

// a trait to identity this type and std::vector
template <typename T> struct is_a_vector                  : mpl::false_{};
template <>           struct is_a_vector<my_vector>       : mpl::true_ {};
template <typename T> struct is_a_vector<std::vector<T> > : mpl::true_ {};

struct algebra_crochet { 

 // how to wrap the scalar into the unity of the algebra
 template<typename T> struct wrap_scalar {
  T s; 
  wrap_scalar( T const & x) : s(x) {}
  double operator[](size_t i) const { return s;}
  size_t size() const {return 0;}
 };

 // how to make the various operations 
 template<typename ProtoTag, typename L, typename R> struct wrap_binary_node {
  L const & l; R const & r;
  wrap_binary_node (L const & l_, R const & r_):l(l_),r(r_) {} 
  double operator[](size_t i) const { return triqs::utility::proto::_ops_<ProtoTag, double,double>::invoke (l[i] , r[i]);}
  size_t size() const {return std::max(l.size(),r.size());}
 };

 // how to make the unary -
 template<typename T> struct wrap_negate {
  T const & s; 
  wrap_negate(T const & x) : s(x) {}
  double operator[](size_t i) const { return s[i];}
  size_t size() const {return s.size();}
 };
};

struct my_algebra_desc : algebra_crochet { 

 // the trait to identify the scalar
 template <typename T> struct is_scalar : triqs::utility::proto::is_in_ZRC<T> {};

 // the trait to identify the elements of the algebra
 template <typename T> struct is_element : is_a_vector<T> {};
};


 template <typename Expr> struct The_Expr;
 typedef triqs::utility::proto::algebra::grammar_generator<my_algebra_desc,The_Expr>::type grammar;
 typedef triqs::utility::proto::algebra::domain<grammar,The_Expr,false> domain;

 template<typename Expr> struct The_Expr : boost::proto::extends<Expr, The_Expr<Expr>, domain>{
  typedef boost::proto::extends<Expr, The_Expr<Expr>, domain> basetype;
  The_Expr( Expr const & expr = Expr() ) : basetype ( expr ) {}

  typedef typename boost::result_of<grammar(Expr) >::type _G;

  double operator[] (size_t n) const { return grammar()(*this)[n]; }
  size_t size() const { return grammar()(*this).size(); }

  //friend std::ostream &operator <<(std::ostream &sout, The_Expr<Expr> const &expr) { return boost::proto::eval(expr, triqs::utility::proto::AlgebraPrintCtx (sout)); }

 };

 BOOST_PROTO_DEFINE_OPERATORS(is_a_vector, domain);

 int main() { 

  my_vector V1, V2;
  std::vector<double> V3(3,2.3);

  V1[0]=1;V1[1]=2;V1[2]=3;
  V2[0]=10;V2[1]=20;V2[2]=30;

  //TEST( (V1 - V2));

  TEST( (2* V1 + V2) [0]);
  TEST( (2* V1 + V2) [1]);
  TEST( (2* V1 + V2) [2]);

  TEST( (V1 - V2).size());

  TEST( (2* V1 + V3) [1]);

 };




