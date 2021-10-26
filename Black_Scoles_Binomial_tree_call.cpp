#include <iostream>
#include <fstream>
#include <cmath>

using namespace std;

//Function to load the file and find the number of lines
void load_file(ifstream &file,int *plines){
    string name;
    cout << "Enter file name: ";
    cin >> name;
    file.open(name);
    string entry;
    while(getline(file,entry)){
        *plines=*plines+1;
    }
    file.clear();
    file.seekg(0);
}

//Function to calculate the volatilities
void volatility_function(float *stockPrices,float *vols,int lines,int period){
    float av=0;     //Variable for calculate the average
    float sdSum=0;  //Variable for the standard deviation
    for(int i=period-1;i<lines;++i){
        av=0;
        for(int j=i-period+1;j<i+1;++j){
            av=av+stockPrices[j];
        }
        av=av/period;
        sdSum=0;
        for(int j=i-period+1;j<i+1;++j){
            sdSum=sdSum+pow(stockPrices[j]-av,2);
        }
        vols[i+1-period]=(pow(sdSum/period,0.5))/stockPrices[i];
    }
}

//Writing parameters into array function
void table_function(float **table,int col,int startPoint,float *data,int nValid){
    for(int i=0;i<nValid;++i){
        table[i][col]=data[i+startPoint];
    }
}

//Function to calculate the d1 values
void d1_function(float **table,int nValid,float *d1){
    for(int i=0;i<nValid;++i){
        d1[i]=(log(table[i][0]/table[i][4])+((table[i][3]+(pow(table[i][1],2)/2))*table[i][2]))/(table[i][1]*pow(table[i][2],0.5));
    }
}

//Function to calculate the d2 values
void d2_function(float **table,int nValid,float *d2){
    for(int i=0;i<nValid;++i){
        d2[i]=table[i][5]-table[i][1]*pow(table[i][2],0.5);
    }
}

//CDF of normal distribution
float CDF_function(float value){
    float av=0;
    for(float i=-100;i<value;i=i+0.001){
        av=av+exp(-pow(i,2)/2);
    }
    return av*0.39894228*0.001;

}

//Call option price
void Call_function(float **table,int nValid,float *C){
    for(int i=0;i<nValid;++i){
        C[i]=table[i][7]*table[i][0]-table[i][8]*table[i][4]*exp(-table[i][3]*table[i][2]);
    }
}

//up multiplier function
void u_function(float **table,int nValid,float *u,int steps){
    for(int i=0;i<nValid;++i){
        u[i]=exp(table[i][1]*pow(table[i][2]/(steps-1),0.5));
    }
}

//Down multiplier function
void d_function(float **table,int nValid,float *d){
    for(int i=0;i<nValid;++i){
        d[i]=1/table[i][10];
    }
}

//Binomial tree function
void binomial_function(float **table,int nValid,float *Cb,int steps,float **tree){
    //Array to calculate the prices
    float Ctree[steps][steps];
    //For projecting back the prices
    float p;

    for(int i=0;i<nValid;++i){

        //Creating the binomial tree for the specific i
        for(int j=0;j<steps;++j){
            for(int jj=0;jj<steps;++jj){
                tree[j][jj]=0;
            }
        }

        tree[0][0]=table[i][0];
        for(int j=0;j<steps-1;++j){
            for(int jj=0;jj<j+1;++jj){
                tree[j+1][jj]=tree[j][jj]*table[i][11];
                tree[j+1][jj+1]=tree[j][jj]*table[i][10];
            }
        }

        //Wiping the price array and assigning the final values
        for(int ii=0;ii<steps;++ii){
            for(int j=0;j<steps;++j){
                if(ii==steps-1&&tree[ii][j]-table[i][4]>0){
                    Ctree[ii][j]=tree[ii][j]-table[i][4];
                }else{
                    Ctree[ii][j]=0;
                }
            }
        }


        //Finding the call option price
        p=(exp(table[i][3]*(table[i][2]/(steps-1)))-table[i][11])/(table[i][10]-table[i][11]);
        for(int j=steps-2;j>-1;j=j-1){
            for(int jj=0;jj<j+1;++jj){
                Ctree[j][jj]=exp(-table[i][3]*(table[i][2]/(steps-1)))*(Ctree[j+1][jj]*(1-p)+Ctree[j+1][jj+1]*p);
            }
        }
        /*
        for(int j=0;j<steps;++j){
            for(int jj=0;jj<steps;++jj){
                cout << Ctree[j][jj] << " ";
            }
            cout << endl;
        }
        cout << endl;
        for(int j=0;j<steps;++j){
            for(int jj=0;jj<steps;++jj){
                cout << tree[j][jj] << " ";
            }
            cout << endl;
        }
        */

        Cb[i]=Ctree[0][0];

    }
}


int main(){
    ifstream file;              //file variable
    int lines=0;                //Number of lines in file variable
    int *plines=&lines;         //Number of lines in file pointer
    load_file(file,plines);     //Loading the data into an array and finding the number of lines

    float *stockPrices;
    stockPrices=new float [lines];          //Array for the stock prices
    for(int i=0;i<lines;++i){               //Loading data into array
        file >> stockPrices[i];
    }

    file.close();               //Closing file

    //All external data has now been loaded in
    int period;
    cout << "Enter the period for the volatility calculation: ";
    cin >> period;
    int nValid=lines-period+1;    //The number of points that will have option prices associated

    float *vols;                //Array for the volatilities
    vols=new float [nValid];

    volatility_function(stockPrices,vols,lines,period); //Calculating the volatilites



    float **table;                  //Creating the array for prices matched to volatilities
    table=new float *[nValid];
    int num=13;                      //The number of columns needed for the array
    for(int i=0;i<nValid;++i){
        table[i]=new float [num];
    }

    for(int i=0;i<num;++i){      //Setting to zero
        for(int j=0;j<nValid;++j){
            table[j][i]=0;
        }
    }

    //Using table function to write the data to an array to make it easier to use
    table_function(table,0,period-1,stockPrices,nValid);
    table_function(table,1,0,vols,nValid);


    //Calculating the time till maturity at each point
    //Assumes that maturity is just after the end of the data
    float timeLeft[nValid];
    int days=1;
    for(int i=nValid;i>-1;i=i-1){
        timeLeft[i]=(1.0/252)*days;
        days=days+1;
    }
    //Writing the time to maturity into the table
    table_function(table,2,0,timeLeft,nValid);


    //Risk free rate
    float r[nValid];
    float rate;
    cout << "Enter the risk free interest rate: ";
    cin >> rate;
    for(int i=0;i<nValid;++i){
        r[i]=rate;
    }
    //Writing the risk free interest rate into the table
    table_function(table,3,0,r,nValid);


    //Strike price
    float K[nValid];
    float strike;
    cout << "Enter the strike price: ";
    cin >> strike;
    for(int i=0;i<nValid;++i){
        K[i]=strike;
    }
    //Writing the strike price into the table
    table_function(table,4,0,K,nValid);

    //Steps variable for the binomial tree
    int steps;
    cout << "Enter the number of steps for all trees: ";
    cin >> steps;
    steps=steps+1;


    //d1 array
    float *d1;
    d1=new float [nValid];

    //Calculating d1 values
    d1_function(table,nValid,d1);
    //Writing the array to the table
    table_function(table,5,0,d1,nValid);


    //d2 array
    float *d2;
    d2=new float [nValid];
    //Calculating d2 values
    d2_function(table,nValid,d2);
    //Writing the array to the table
    table_function(table,6,0,d2,nValid);


    //Nd1 array
    float *Nd1;
    Nd1=new float [nValid];
    //Calculating Nd1 values
    for(int i=0;i<nValid;++i){
        Nd1[i]=CDF_function(table[i][5]);
    }
    //Writing the array to the table
    table_function(table,7,0,Nd1,nValid);


    //Nd2 array
    float *Nd2;
    Nd2=new float [nValid];
    //Calculating Nd2 values
    for(int i=0;i<nValid;++i){
        Nd2[i]=CDF_function(table[i][6]);
    }
    //Writing the array to the table
    table_function(table,8,0,Nd2,nValid);


    //Call option price
    float *C;
    C=new float [nValid];
    //Calculating the call option prices
    Call_function(table,nValid,C);
    //Writing the array to the table
    table_function(table,9,0,C,nValid);


    //Black Scholes part is now done



    //The start of the binomial tree section

    //up multiplier
    float *u;
    u=new float [nValid];
    //Calculating the u for each entry
    u_function(table,nValid,u,steps);
    //Writing the array to the table
    table_function(table,10,0,u,nValid);


    //down multiplier
    float *d;
    d=new float [nValid];
    d_function(table,nValid,d);
    //Writing the array to the table
    table_function(table,11,0,d,nValid);

    //Array for the tree at each stage
    float **tree;
    tree=new float *[steps];
    for(int i=0;i<steps;++i){
        tree[i]=new float [steps];
    }
    for(int i=0;i<steps;++i){
        for(int j=0;j<steps;++j){
            tree[i][j]=0;
        }
    }


    //Array for the prices from the binomial tree function
    float *Cb;
    Cb=new float [nValid];
    binomial_function(table,nValid,Cb,steps,tree);
    //Writing the array to the table
    table_function(table,12,0,Cb,nValid);


    //Writing all of the data to the console and a csv file
    ofstream output("output.csv");
    output<<"S,"<<"v,"<<"t,"<<"r,"<<"K,"<<"d1,"<<"d2,"<<"Nd1,"<<"Nd2,"<<"C,"<<"u,"<<"d,"<<"Cb"<<endl;
    for(int i=0;i<nValid;++i){
        for(int j=0;j<num;++j){
            output << table[i][j] << ",";
        }
        output << endl;
    }

    return 0;
}
