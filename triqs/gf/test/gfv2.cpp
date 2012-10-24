//#define TRIQS_ARRAYS_ENFORCE_BOUNDCHECK

#include <triqs/gf/matsubara.hpp> 

namespace tql= triqs::clef;
namespace tqa= triqs::arrays;
using tqa::range;
using triqs::arrays::make_shape;
using triqs::gf::Fermion;
using triqs::gf::matsubara_freq;
using triqs::gf::matsubara_time;

#define TEST(X) std::cout << BOOST_PP_STRINGIZE((X)) << " ---> "<< (X) <<std::endl<<std::endl;

int main() {

 triqs::gf::freq_infty inf;

 triqs::gf::gf<triqs::gf::matsubara_freq> G1; // empty
 TEST( G1( 0) ) ;

 double beta =1;
 auto G = make_gf (matsubara_freq(), beta, Fermion, make_shape(2,2));
 auto Gc= make_gf (matsubara_freq(), beta, Fermion, make_shape(2,2));
 auto G3= make_gf (matsubara_freq(), beta, Fermion, make_shape(2,2));
 auto Gt= make_gf (matsubara_time(), beta, Fermion, make_shape(2,2));

 //triqs::gf::gf_view<triqs::gf::matsubara_freq> Gv =G;
 auto Gv = G();
 TEST( G( 0) ) ;
 Gv(0) = 20;
 TEST( Gv( 0) ) ;
 TEST( G( 0) ) ;
 Gv(0) = 0;

 auto Gv2 = slice_target(G,range(0,1),range(0,1));
 TEST( Gv2( 0) ) ;
 Gv2(0) = 10;
 TEST( Gv2( 0) ) ;
 TEST( G( 0) ) ;

 triqs::clef::placeholder<0> om_;

 TEST( G(om_) ) ;
 TEST( tql::eval(G(om_), om_=0) ) ;

 TEST( Gv(om_) ) ;
 TEST( tql::eval(Gv(om_), om_=0) ) ;

 std::cout  <<"-------------lazy assign ------------------"<<std::endl;

 Gv(om_) = (0.2 + om_ + 2.1);
 TEST(G(0));
 TEST(G(inf));

 std::cout  <<"-------------lazy assign ------------------"<<std::endl;

 G(om_) = 1/(om_ + 2.3);

 TEST(G(0));
 TEST(G(inf));
 TEST(inverse(G(inf)));

 std::cout  <<"-------------------------------------"<<std::endl;

#define SUITE
#ifdef SUITE

 TEST( Gv(om_) ) ;
 TEST( tql::eval(Gv(om_), om_=0) ) ;

 // tail 
 BOOST_AUTO( t, G(inf));
 //local::gf<meshes::tail> t2 = t + 2.4;

 TEST(t.order_min()); 
 TEST( t( 0) ) ;

 TEST( Gv2(inf)( 0) ) ;

 // copy 
 Gc = G;
 TEST( G( 0) ) ;
 TEST( Gc( 0) ) ;


 // operations on gf
 G3 = G + Gc;

 // should not compile
 //G3 = G + Gt;

 //#define ALL_TEST
#ifdef ALL_TEST
 for (int u=0; u<10; ++u) { 
  TEST( (G + 2.0* Gc)( u) ) ;
  TEST( (8.0*G + 2.0* Gc)( u) ) ;
  TEST( (8.0*G  - 2.0* Gc)( u) ) ;
  TEST( (G - Gc)( u) ) ;
  TEST( (G - 2.0* Gc)( u) ) ;
  TEST( (G * Gc)( u) ) ;
 }
#endif
 TEST( G( 0) ) ;
 TEST(G(inf)(0));

 TEST( ( G(inf) + G(inf) )  (0));
 TEST( ( G(inf) * G(inf) )  (0));
 TEST( (G + Gc)( inf) ) ;

 //TEST( (G + 2.3)(0));
 TEST( (t + 2.3)(0));

 TEST( t(-1));
 //TEST( (2 * inverse(t))(0));

 tqa::array<double,9> A(1,2,3,4,5,6,7,8,9);
 A()=0;
 //auto x = local::impl::gf_impl<triqs::gf::meshes::matsubara_freq, true>::wrap_infty (G.tail_view()) + 2.0;

 // test hdf5 
 H5::H5File file("ess_gf.h5", H5F_ACC_TRUNC );
 h5_write(file, "g", G);
#endif
}