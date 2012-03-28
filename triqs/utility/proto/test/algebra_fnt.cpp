#include <triqs/utility/proto/algebra.hpp>
#include <boost/type_traits/is_complex.hpp> 
#include <triqs/arrays/array.hpp>
#include <triqs/arrays/matrix.hpp>
#include <triqs/arrays/expressions/matrix_algebra.hpp>
//#include <triqs/arrays/expressions/array_algebra.hpp>
#include <vector>
#include <iostream>

namespace mpl = boost::mpl;
using triqs::arrays::matrix;
using triqs::arrays::range;
using triqs::arrays::array;

#define TEST(X) std::cout << BOOST_PP_STRINGIZE((X)) << " ---> "<< (X) <<std::endl;

struct my_matrix_valued_function {
 triqs::arrays::array<double, 3> data;
 my_matrix_valued_function (double x): data(2,2,5) { data()=0; for (size_t u=0; u< 5; ++u) {data(0,0,u) = x*u;data(1,1,u) = -x*u;}}
 my_matrix_valued_function(my_matrix_valued_function const &x): data(x.data) { std::cerr  << "COPY my_matrix_valued_function"<<std::endl ; }
 triqs::arrays::matrix_view<double> operator()(size_t i) const { return data(range(),range(),i);}
};

// a trait to identity this type 
template <typename T> struct is_a_m_f                            : mpl::false_{};
template <>           struct is_a_m_f<my_matrix_valued_function> : mpl::true_ {};

// a trait to find the scalar of the algebra i.e. the true scalar and the matrix ...
template <typename T> struct is_scalar_or_element   : mpl::or_< triqs::arrays::expressions::matrix_algebra::IsMatrix<T>, triqs::utility::proto::is_in_ZRC<T> > {};


namespace tupa=triqs::utility::proto::algebra;

template <typename Expr> struct The_Expr;  // the expression

typedef tupa::grammar_generator<tupa::algebra_function_desc,is_a_m_f, is_scalar_or_element>::type grammar; // the grammar

typedef tupa::domain<grammar,The_Expr,true>  domain; // the domain 

template<typename Expr> struct The_Expr : boost::proto::extends<Expr, The_Expr<Expr>, domain>{ // impl the expression
 typedef boost::proto::extends<Expr, The_Expr<Expr>, domain> basetype;
 The_Expr( Expr const & expr = Expr() ) : basetype ( expr ) {}
 typedef typename boost::result_of<grammar(Expr) >::type _G;
 typename triqs::utility::proto::call_result_type<_G,size_t>::type operator() (size_t n) const { return grammar()(*this)(n); }
 friend std::ostream &operator <<(std::ostream &sout, The_Expr<Expr> const &expr) { return boost::proto::eval(expr, triqs::utility::proto::AlgebraPrintCtx (sout)); }
};

BOOST_PROTO_DEFINE_OPERATORS(is_a_m_f, domain);

int main() { 

 my_matrix_valued_function f1(1),f2(10);
 triqs::arrays::matrix<double> A(2,2); A()=0; A(0,0) = 2; A(1,1) = 20;

 TEST( (f1 - f2));

 TEST(f1(1));

 TEST( (2.0* f1 ) (0));
 TEST( (2* f1 + f2) (1));

 TEST( triqs::arrays::matrix<double> ( ( 2*f1  +f2 ) (1)) );
 TEST( triqs::arrays::eval ( ( 2*f1  +f2 ) (1)) );

 TEST( triqs::arrays::eval ( ( A*f1  +f2 ) (1)) );

};

