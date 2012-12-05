class ctx :
    def __init__ (self, G) : 
        self.G = G
    def _is_compatible_for_ops(self, g): 
        m1,m2  = self.G._mesh, g._mesh
        return m1 is m2 or m1 == m2
    def __eq__ (self, y) :
        return isinstance(y, self.__class__) and self._is_compatible_for_ops(y.G)
    def __call__ (self, x) : 
        if not isinstance(x, Descriptors.Base) : return x
        tmp = self.G.copy()
        x(tmp)
        return tmp

def ilshift_impl(self, A): 
    """ A can be two things :
      * G <<= any_GF_Initializers will init the GFBloc with the initializer
      * G <<= g2 where g2 is a GFBloc will copy g2 into self
    """

    if isinstance(A, self.__class__) : 
        if self is not A : self.copyFrom(A) # otherwise it is useless AND does not work !!
    elif isinstance(A, lazy_expressions.lazy_expr) : # A is a lazy_expression made of GF, scalars, descriptors 
        A2= Descriptors.convert_scalar_to_Const(A)
        def e_t (x) : 
            if not isinstance(x, Descriptors.Base) : return x
            print x, self._tail
            tmp = self.copy()
            print tmp._tail
            x(tmp)
            print "final ", tmp._tail
            return tmp
        #e_t2 = self.__lazy_expr_eval_context__()
        self.copyFrom ( lazy_expressions.eval_lazy_expr(e_t, A2) )
    elif isinstance(A, lazy_expressions.lazy_expr_terminal) : #e.g. g<<= SemiCircular (...) 
        self <<= lazy_expressions.lazy_expr(A)
    elif Descriptors.is_scalar(A) : #in the case it is a scalar .... 
        self <<= lazy_expressions.lazy_expr(A)
    elif isinstance(A, GF_Initializers.Base) : # backwards compatibility, deprecated
        A(self)
    else :
        raise RuntimeError, " GF Block : <<= operator : RHS not understood"
    return self
