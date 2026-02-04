/*
ptnSingleton.h
Author: Mars
Date: 2005-3-4
单件模式通用模板
*/
#ifndef __PTNSINGLETON__
#define __PTNSINGLETON__
//namespace marsCore
//{
	template<class T>
	class ptnSingleton
	{
			friend ptnSingleton;
		public:
			static T& instance() 
			{
				static T theInstance; 
				return theInstance;
			}
		protected:
			ptnSingleton() {}
			virtual ~ptnSingleton() {}

		private:
			ptnSingleton(const ptnSingleton&);
			ptnSingleton& operator=(const ptnSingleton&);
	};
//} // marsCore
#endif // __PTNSINGLETON__
