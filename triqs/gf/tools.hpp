/*******************************************************************************
 *
 * TRIQS: a Toolbox for Research in Interacting Quantum Systems
 *
 * Copyright (C) 2012 by M. Ferrero, O. Parcollet
 *
 * TRIQS is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * TRIQS is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * TRIQS. If not, see <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/
#ifndef TRIQS_GF_TOOLS_H
#define TRIQS_GF_TOOLS_H
#include <utility>
#include <boost/iterator/iterator_facade.hpp>
#include <triqs/clef/core.hpp>
#include <triqs/arrays/array.hpp>
#include <triqs/arrays/matrix.hpp>
#include <triqs/utility/proto/tools.hpp>
#include <triqs/arrays/h5/simple_read_write.hpp>
#include <triqs/arrays/proto/matrix_algebra.hpp>
#include <triqs/arrays/proto/array_algebra.hpp>
#include "triqs/utility/complex_ops.hpp"
#include <triqs/utility/view_tools.hpp>

namespace triqs { namespace gf {

 namespace tqa= triqs::arrays;
 namespace mpl=boost::mpl;
 namespace proto=boost::proto;
 namespace bf = boost::fusion; 
 namespace tup = triqs::utility::proto;

 //------------------------------------------------------

 typedef std::complex<double> dcomplex; 

 enum statistic_enum {Boson,Fermion};

 struct freq_infty{}; // the point at infinity

 struct zero_t{}; struct one_t{}; struct two_t{}; struct three_t{};// to dispatch index_point in mesh...
 
 //------------------------------------------------------

 struct nothing {
  template<typename... Args> nothing(Args...) {} // takes anything, do nothing..
  nothing() {}
  typedef void has_view_type_tag;     // Idiom : ValueView  
  typedef nothing view_type;
  typedef nothing non_view_type;
   
  friend void h5_write (tqa::h5::group_or_file fg, std::string subgroup_name, nothing ) {}
  friend void h5_read  (tqa::h5::group_or_file fg, std::string subgroup_name, nothing ) {}
 }; 

 //------------------------------------------------------

 // Derive from this object using CRTP to provide arithmetic operation by casting the final object to C 
 template<typename Derived, typename C> struct arith_ops_by_cast {};
#define IMPL_OP(OP)\
 template<typename D, typename C, typename T> \
 auto operator OP(arith_ops_by_cast<D,C> const & x, T y) -> decltype( std::declval<C>() OP y) {return C(static_cast<D const& >(x)) OP y;}\
 template<typename D, typename C, typename T> \
 auto operator OP(T y, arith_ops_by_cast<D,C> const & x) -> decltype (y OP std::declval<C>()) {return y OP C(static_cast<D const& >(x));}
 IMPL_OP(+); IMPL_OP(-); IMPL_OP(*); IMPL_OP(/);
#undef IMPL_OP

 //------------------------------------------------------

 /// The wrapper for the mesh point
 template<typename MeshType>
  struct mesh_point_t : arith_ops_by_cast<mesh_point_t<MeshType>, typename MeshType::domain_t::point_t  > {
   typedef MeshType mesh_t;
   mesh_t const * m;  
   typename mesh_t::index_t index; 
   mesh_point_t( mesh_t const & mesh, typename mesh_t::index_t const & index_=0): m(&mesh), index(index_) {}
   void advance() { ++index;}
   operator typename MeshType::domain_t::point_t () const { return m->index_to_point(index);} 
  };

 //------------------------------------------------------
 /*
 /// The wrapper for the mesh point
 template<typename MeshType>
  struct mesh_point_N_t { 
   typedef MeshType mesh_t;
   const mesh_t * m; 
   typename mesh_t::index_t index;

   /// The wrapper for one component of the mesh point
   template<typename Order> struct component : arith_ops_by_cast<component<Order>, double>  { 
    mesh_point_N_t const * p; 
    component (mesh_point_t const & p_):p(&p_){}
    operator double() const { return p.m->index_to_point(p->index,Order());}
   };
   component<zero_t> _0;
   component<one_t>  _1;

   mesh_point_t(mesh_t const & m_, index_t index_ ) : m(&m_), index(index_), t0(*this), t1(*this) { }
   mesh_point_t(mesh_t const & m_) : m(&m_), index{{0,0}}, t0(*this), t1(*this) { }

   void advance() { ++index[0]; if (index[0]==m->L) { index[0]=0; ++index[1];}}
   operator typename MeshType::domain_t::point_t() const { return m->index_to_point(index);}
  };
  */
 //------------------------------------------------------

 template<typename MeshType>
  class mesh_pt_generator : 
   public boost::iterator_facade< mesh_pt_generator<MeshType>, typename MeshType::mesh_point_t const &, boost::forward_traversal_tag, 
   typename MeshType::mesh_point_t const & > {
    friend class boost::iterator_core_access;
    MeshType const * mesh;
    size_t u;
    typename MeshType::mesh_point_t pt;
    typename MeshType::mesh_point_t const & dereference() const { return pt;}
    bool equal(mesh_pt_generator const & other) const { return ((mesh == other.mesh) && (other.u==u) );}
    public:
    mesh_pt_generator( MeshType const * m=NULL, bool atEnd = false): mesh(m), u(atEnd ? m->size(): 0), pt(*m) {}
    //~mesh_pt_generator() { std::cout  << "deleting generator" << std::endl; }
    void increment() { ++u; pt.advance(); }
    bool at_end() const { return (u>=mesh->size()-1);}
    typename MeshType::domain_t::point_t to_point() const { return pt;}    
   };
}}
#endif