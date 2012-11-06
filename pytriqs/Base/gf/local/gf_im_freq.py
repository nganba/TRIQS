################################################################################
#
# TRIQS: a Toolbox for Research in Interacting Quantum Systems
#
# Copyright (C) 2011-2012 by M. Ferrero, O. Parcollet
#
# TRIQS is free software: you can redistribute it and/or modify it under the
# terms of the GNU General Public License as published by the Free Software
# Foundation, either version 3 of the License, or (at your option) any later
# version.
#
# TRIQS is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# TRIQS. If not, see <http://www.gnu.org/licenses/>.
#
################################################################################

__all__ = ['GfImFreq']
import numpy
from math import pi
from gf import GfImFreq_cython, MeshImFreq  
from gf_local_general import GfLocalGeneral 

class GfImFreq (GfImFreq_cython, GfLocalGeneral):
    """ 
    A matrix-valued block Green's function in Matsubara frequencies.
    """
    hdf5_scheme_doc = {'data' : "The array of data"}
    
    def __init__(self, **d):
        """
     The constructor have two variants : you can either provide the mesh in
     Matsubara frequencies yourself, or give the parameters to build it.
     All parameters must be given with keyword arguments.

     GfImFreq(Indices, Beta, Statistic, NFreqMatsubara,  Data, Tail, Name,Note)

           * ``Indices``:  a list of indices names of the block
           * ``Beta``:  Inverse Temperature 
           * ``Statistic``:  GF_Statistic.Fermion [default] or GF_Statistic.Boson
           * ``NFreqMatsubara``:  Number of Matsubara frequencies
           * ``Data``:   A numpy array of dimensions (len(Indices),len(Indices),NFreqMatsubara) representing the value of the Green function on the mesh. 
           * ``Tail``:  the tail 
           * ``Name``:  a name of the GF
           * ``Note``:  any string you like...

     If you already have the mesh, you can use a simpler version :

     GfImFreq(Indices, Mesh, Data, Tail, Name,Note)
        
           * ``Indices``:  a list of indices names of the block
           * ``Mesh``:  a MeshGF object, such that Mesh.TypeGF== GF_Type.Imaginary_Frequency 
           * ``Data``:   A numpy array of dimensions (len(Indices),len(Indices),NFreqMatsubara) representing the value of the Green function on the mesh. 
           * ``Tail``:  the tail 
           * ``Name``:  a name of the GF
           * ``Note``:  any string you like...

.. warning::
    The Green function take a **view** of the array Data, and a **reference** to the Tail.

         """
        # construct the mesh if needed
        if 'Mesh' not in d : 
            if 'Beta' not in d : raise ValueError, "Beta not provided"
            Beta = float(d.pop('Beta'))
            Nmax = d.pop('NFreqMatsubara',1025)
            stat = d.pop('Statistic','F') # GF_Statistic.Fermion
            sh = 1 if stat== 'F' else 0 # GF_Statistic.Fermion else 0
            d['Mesh'] = MeshImFreq(Beta,'F',Nmax)
            #d['Mesh'] = MeshImFreq(Beta,GF_Statistic.Fermion,Nmax)

        GfImFreq_cython.__init__(self,*self._prepare_init(d))
        
    #-----------------------------------------------------

    def _plot_(self, OptionsDict):
        """ Plot protocol. OptionsDict can contain : 
             * :param RIS: 'R', 'I', 'S', 'RI' [ default] 
             * :param x_window: (xmin,xmax) or None [default]
             * :param Name: a string [default ='']. If not '', it remplaces the name of the function just for this plot.
        """
        M = [x.imag for x in self._mesh]        
        #M = [self._mesh.get_point(i).imag for i in range(self._mesh.size())]
        return self._plot_base( OptionsDict,  r'$\omega_n$', lambda name : r'%s$(i\omega_n)$'%name, True, M)
    
    #-----------------------------------------------------

    def fitTail(self, fixed_coef, order_max, fit_start, fit_stop, replaceTail = True):
       """
       Fit the tail of the Green's function
       Input:
         - fixed_coef: a 3-dim array of known coefficients for the fit starting from the order -1
         - order_max: highest order in the fit
         - fit_start, fit_stop: fit the data between fit_start and fit_stop
       Output:
         On output all the data above fit_start is replaced by the fitted tail
         and the new moments are included in the GF
       """
       from scipy.optimize import leastsq

       # Turn known_coefs into a numpy array if ever it is not already the case
       #known_coef = numpy.array(fixed_coef)
       known_coef = fixed_coef

       # Check the shape of the known_coef
       #assert(known_coef.shape[0:2] == (self.N1,self.N2))

       # Change the OrderMax
       # It is assumed that any known_coef will start at order -1
       self._tail.changeOrderMax(order_max)

       # Fill up two arrays with the frequencies and values over the range of interest
       ninit, nstop = 0, -1
       x = []
       for om in self.mesh:
         if (om.imag < fit_start): ninit = ninit+1
         if (om.imag <= fit_stop): nstop = nstop+1
         if (om.imag <= fit_stop and om.imag >= fit_start): x += [om]
       omegas = numpy.array(x)
       #values = self._data[:,:,ninit:nstop+1].array
       values = self._data.array[:,:,ninit:nstop+1]

       # Loop over the indices of the GF
       for n1,indR in enumerate(self._IndicesR):
         for n2,indL in enumerate(self._IndicesL):
           
           # Construct the part of the fitting functions which is known
           f_known = numpy.zeros((len(omegas)),numpy.complex)
           for order in range(len(known_coef[n1][n2])):
             f_known += known_coef[n1][n2][order]*omegas**(1-order)
	   
           # Compute how many free parameters we have and give an initial guess
           len_param = order_max-len(known_coef[n1][n2])+2
           p0 = len_param*[1.0]
	   
           # This is the function to be minimized, the diff between the original
           # data in values and the fitting function
           def fct(p):
             y_fct = 1.0*f_known
             for order in range(len_param):
               y_fct += p[order]*omegas**(1-len(known_coef[n1][n2])-order)
             y_fct -= values[n1,n2,:]
             return abs(y_fct)

           # Now call the minimizing function
           sol = leastsq(fct, p0, maxfev=1000*len_param)

           # Put the known and the new found moments in the tail
           for order in range(len(known_coef[n1][n2])):
             self._tail[order-1][indR,indL] = [[ known_coef[n1][n2][order] ]]
           for order, moment in enumerate(sol[0]):
             self._tail[len(known_coef[n1][n2])+order-1][indR,indL] = [[ moment ]]

       # Replace then end of the GF by the tail
       if replaceTail: self.replaceByTail(ninit);


#-----------------------------------------------------
#  Register the class for HDF_Archive
#-----------------------------------------------------

from pytriqs.Base.Archive.HDF_Archive_Schemes import register_class
register_class (GfImFreq)

