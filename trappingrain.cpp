class Solution {
public:
    int trap(vector<int>& height) {
        int n=height.size(),i=0,j=n-1;
    int maximum=*max_element(height.begin(), height.end());
    int rightmaximum=0,k,m,p,q=0,trap1=0,trap2=0,leftmaximum=0,trap=0;
        while(leftmaximum<maximum){
            if(height[i]>=leftmaximum){
                leftmaximum = height[i];
                m=i;
                i++;
                continue;
            }
            else if(height[i]<leftmaximum){
                trap1=trap1+height[m]-height[i];
                i++;
            }
        }
        while(rightmaximum<maximum){
            if(height[j]>=rightmaximum){
                rightmaximum = height[j];
                k=j;
                j--;
                continue;
            }
            else if(height[j]<rightmaximum){
                trap2=trap2+height[k]-height[j];
                j--;
            }
            
        }
        int max=0;
        for(int i=0;i<n;i++){
            if(height[i]==maximum){
                q++;
            }
        }
        if(q>1){
            for(int i=m;i<k;i++){
                    if(height[i]>=max){
                        max=height[i];
                        p=i;
                        continue;
                    }
                    else{
                        trap=trap+height[p]-height[i];
                    }
                
            }
            return trap+trap1+trap2;
        }
        return trap1+trap2;
    }
};

        for(int i=0;i<n;i++){
            if(a[i]<0){
                a[i]=0-a[i];
            }
            else{
                int root=sqrt(a[i]);
                a[i]= min( a[i] - root*root  , (root+1)*(root+1) - a[i]  )
            }
        }
        sort(a.begin(),a.end());
        return *accumulate(a.begin(),a.end()-n/2)

        int mini= *min_element(a.begin(),a.end());
        // n..-> 
         while(n){
            v.push_back(n%10);
            n=n/10;
        }
        
        for(int i = v.size()-1;i>0;i--){
            
        }