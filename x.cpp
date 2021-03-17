#include <math.h>
#include <iostream>
#include <iomanip>

using namespace std;

int main()
{
	double R = 2.0 * sqrt(3.0) / (2.0 + sqrt(3.0) + sqrt(7.0));
	cout << R << endl;

	double S = sqrt(R*R + (2-R)*(2-R)) + sqrt(2.0)*R + sqrt(R*R + (sqrt(3)-R)*(sqrt(3)-R));

	cout << setprecision(20) << S << endl;
	cout << setprecision(20) << S*S << endl; 

	cout << "Sum of sq: " << (R*R + (2-R)*(2-R) + 2*R*R + R*R + (sqrt(3)-R)*(sqrt(3)-R)) << endl;

	double K = (sqrt(7.0)/3.0 + 4.0/3.0 + sqrt(19)/3.0);

	cout << K << endl;
	cout << K*K << endl;

	return 0;
}
