#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <utility>
#include <cassert>
#include "ec_ops.h"
using namespace std;

Zp Zp::inverse() const{
	// Implement the Extended Euclidean Algorithm to return the inverse mod PRIME
	uberzahl a = value;
	uberzahl b = PRIME;
	uberzahl x = uberzahl(1);
	uberzahl y = uberzahl(0);
	uberzahl xLast = uberzahl(0);
	uberzahl yLast = uberzahl(1);
	uberzahl q, r, s, t;

   while (a != uberzahl(0))
   {
       q = b / a;
       r = b % a;
       s = xLast - q*x;
       t = yLast - q*y;
       xLast = x;
       yLast = y;
       x = s;
       y = t;
       b = a;
       a = r;
   }		

	return xLast;
}


ECpoint ECpoint::operator + (const ECpoint &a) const {
	// Implement  elliptic curve addition 		
	
	if (this->infinityPoint)
		return a;
	if (a.infinityPoint)
		return this;
	Zp lambda;	
	if ((*this == a)&&(!(this->y * Zp(2) == Zp(0))))
	{
		lambda = (Zp(3) * this->x * this->x + Zp(A)) * (Zp(2) * this->y).inverse();	
		lambda = Zp(lambda.getValue() % PRIME);
	}
	else if ((!(this->x == a.x))&&(!(*this == a)))
	{
		lambda = (a.y - this->y) * (a.x - this->x).inverse();
		lambda = Zp(lambda.getValue() % PRIME);
	}
	else
	{
		return ECpoint(true);
	}
	Zp xr = lambda * lambda - this->x - a.x;
	xr = Zp(xr.getValue() % PRIME);
	Zp yr = -this->y + lambda * (this->x - xr);
	yr = Zp(yr.getValue() % PRIME);
	return ECpoint(xr, yr);
	//assert(0);
	//return ECpoint(true);
}


ECpoint ECpoint::repeatSum(ECpoint p, uberzahl v) const {
	//Find the sum of p+p+...+p (vtimes)		
	if (v == uberzahl(0))
		return ECpoint(true);
	ECpoint temp = p;
	ECpoint total = ECpoint(true);
	uberzahl t = uberzahl(1);
	int i = 0;
	while (t <= v)
	{
		if (((v >> i) & uberzahl(1)) == uberzahl(1))
		{
			total = total + temp;
		}
		i++;
		t = t+t;
		temp = temp + temp;
	}
	return total;
	//assert(0);
	//return ECpoint(true);
}

Zp ECsystem::power(Zp val, uberzahl pow) {
	//Find the product of val*val*...*val (pow times)
	uberzahl c = 0;
	uberzahl f = 1;

	//uberzahl k = 0;
	int k = 0;
	uberzahl j = 1;
	while (j <= pow)
	{
		j = j * uberzahl(2);
		k = k + 1;
	}

	k = k - 1;

	for (int i = k; i >= 0; --i)
	{
		c = c * uberzahl(2);
		f = (f * f) % PRIME;
		if ( ((pow >> i) & uberzahl(1)) == uberzahl(1))
		{
			c = c + uberzahl(1);
			f = (f * val.getValue()) % PRIME;
		}

	} 

	return f;
}


uberzahl ECsystem::pointCompress(ECpoint e) {
	//It is the gamma function explained in the assignment.
	//Note: Here return type is mpz_class because the function may
	//map to a value greater than the defined PRIME number (i.e, range of Zp)
	//This function is fully defined.	
	uberzahl compressedPoint = e.x.getValue();
	compressedPoint = compressedPoint<<1;
	
	if(e.infinityPoint) {
		cout<<"Point cannot be compressed as its INF-POINT"<<flush;
		abort();
		}
	else {
		if (e.y.getValue()%2 == 1)
			compressedPoint = compressedPoint + 1;
		}
		//cout<<"For point  "<<e<<"  Compressed point is <<"<<compressedPoint<<"\n";
		return compressedPoint;

}

ECpoint ECsystem::pointDecompress(uberzahl compressedPoint){
	//Implement the delta function for decompressing the compressed point
	Zp xr = Zp(compressedPoint >> 1);
	Zp z = power(xr, uberzahl(3)) + Zp(A) * xr + Zp(B);
	Zp sqr1 = power(z, (PRIME+1)/4);
	Zp sqr2 = Zp(PRIME) - sqr1;
	if ((sqr1.getValue() & uberzahl(1)) == (compressedPoint & uberzahl(1)))
	{
		return ECpoint(xr, sqr1);
	}
	else
	{
		return ECpoint(xr, sqr2);
	}
	//assert(0);
	//return ECpoint(true);
}


pair<pair<Zp,Zp>,uberzahl> ECsystem::encrypt(ECpoint publicKey, uberzahl privateKey,Zp plaintext0,Zp plaintext1){
	// You must implement elliptic curve encryption
	//  Do not generate a random key. Use the private key that is passed from the main function
	//uberzahl x = random(0, ORDER);
	ECpoint q = privateKey * ECpoint(GX, GY);
	ECpoint r = privateKey * publicKey;
	Zp c0 = Zp((plaintext0 * r.x).getValue() % PRIME);
	Zp c1 = Zp((plaintext1 * r.y).getValue() % PRIME);
	uberzahl c2 = pointCompress(q);
	return make_pair(make_pair(c0, c1), c2);
	//assert(0);
	//return make_pair(make_pair(0,0),0);
}


pair<Zp,Zp> ECsystem::decrypt(pair<pair<Zp,Zp>, uberzahl> ciphertext){
	// Implement EC Decryption
	ECpoint R = privateKey * pointDecompress(ciphertext.second);
	Zp m0 = Zp((ciphertext.first.first * R.x.inverse()).getValue() % PRIME);
	Zp m1 = Zp((ciphertext.first.second * R.y.inverse()).getValue() % PRIME);
	return make_pair(m0, m1);
	//assert(0);
	//return make_pair(0,0);
}


/*
 * main: Compute a pair of public key and private key
 *       Generate plaintext (m1, m2)
 *       Encrypt plaintext using elliptic curve encryption
 *       Decrypt ciphertext using elliptic curve decryption
 *       Should get the original plaintext
 *       Don't change anything in main.  We will use this to 
 *       evaluate the correctness of your program.
 */


int main(void){
	srand(time(0));
	ECsystem ec;
	unsigned long incrementVal;	
	pair <ECpoint, uberzahl> keys = ec.generateKeys();
	
	
	Zp plaintext0(MESSAGE0);
	Zp plaintext1(MESSAGE1);
	ECpoint publicKey = keys.first;
	cout<<"Public key is: "<<publicKey<<"\n";
	
	cout<<"Enter offset value for sender's private key"<<endl;
	cin>>incrementVal;
	uberzahl privateKey = XB + incrementVal;
	
	pair<pair<Zp,Zp>, uberzahl> ciphertext = ec.encrypt(publicKey, privateKey, plaintext0,plaintext1);	
	cout<<"Encrypted ciphertext is: ("<<ciphertext.first.first<<", "<<ciphertext.first.second<<", "<<ciphertext.second<<")\n";
	pair<Zp,Zp> plaintext_out = ec.decrypt(ciphertext);
	
	cout << "Original plaintext is: (" << plaintext0 << ", " << plaintext1 << ")\n";
	cout << "Decrypted plaintext: (" << plaintext_out.first << ", " << plaintext_out.second << ")\n";


	if(plaintext0 == plaintext_out.first && plaintext1 == plaintext_out.second)
		cout << "Correct!" << endl;
	else
		cout << "Plaintext different from original plaintext." << endl;	
			
	return 1;

}


