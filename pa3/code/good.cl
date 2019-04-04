class B {
};

class A inherits B{
	c : Int;
	test2():Int{
		1
	};
};

class C inherits A {
	a : Int;
	b : Bool;
	init(x : Int, y : Bool) : C {
           {
		a <- x;
		b <- y;
		self;
           }
	};
	test():Int{
		1
	};
};

Class Main {
	aa : A <-(new C);
	main():C {
	  {
	    aa.test2();
	    (new C).init(1,true);
	  }
	};
};
