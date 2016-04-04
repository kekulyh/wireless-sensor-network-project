clear
%1.��ʼ�����趨ģ��
%.�������ڵ��������(��λ M)
xm=100;
ym=100;
%(1)��۽��������
sink.x=0.5*xm;
sink.y=0.5*ym;
%�����ڴ�������
n=100
%��ͷ�Ż���������ѡ��ͷ�ĸ��ʣ�
P=0.1;
%����ģ�ͣ���λ ����
%��ʼ������ģ��
Eo=0.5;
%Eelec=Etx=Erx
ETX=50*0.000000001;
ERX=50*0.000000001;
%Transmit Amplifier types
Efs=10*0.000000000001;
Emp=0.0013*0.000000000001;
%Data Aggregation Energy
EDA=5*0.000000001;
%�������ڵ㳬��һ�ڵ������İٷֱ�
a=1;
%���ѭ������
rmax=5000
%������� do
do=sqrt(Efs/Emp);
Et=0;
%2.���ߴ���������ģ�Ͳ���ģ��
%�������ߴ���������,�������ھ���Ͷ��100���ڵ�,������ͼ��
for i=1:1:n
    S3(i).xd=rand(1,1)*xm;
    S4(i).xd=S3(i).xd;
    XR4(i)=S4(i).xd;
    XR3(i)=S3(i).xd;
    S3(i).yd=rand(1,1)*ym;
    S4(i).yd=S3(i).yd;
    YR4(i)=S4(i).yd;
    S4(i).G=0;
    YR3(i)=S3(i).yd;
    S3(i).G=0;
    S3(i).E=Eo*(1+rand*a);
    S4(i).E=S3(i).E;
    E3(i)= S3(i).E;
    E4(i)= S4(i).E;
    Et=Et+E3(i);
    %initially there are no cluster heads only nodes
    S3(i).type='N';
    S4(i).type='N';
end
d1=0.765*xm/2;
K=sqrt(0.5*n*do/pi)*xm/d1^2;
d2=xm/sqrt(2*pi*K);
Er=4000*(2*n*ETX+n*EDA+K*Emp*d1^4+n*Efs*d2^2);
S3(n+1).xd=sink.x;
S3(n+1).yd=sink.y;
S4(n+1).xd=sink.x;
S4(n+1).yd=sink.y;
%3.��������ģ��
%��ͷ�ڵ���
countCHs3=0;
cluster3=1;%�˶����Ŀ�Ľ����Ǹ���һ��1��ʼ���±�����������Ĵ�ͷ��Ӧ�û���ȥ1
flag_first_dead3=0;
flag_teenth_dead3=0;
flag_all_dead3=0;
%�����ڵ���
dead3=0;
first_dead3=0;
teenth_dead3=0;
all_dead3=0;
%��ڵ���
allive3=n;
%counter for bit transmitted to Bases Station and to Cluster Heads
packets_TO_BS3=0;
packets_TO_CH3=0;
%(1)ѭ��ģʽ�趨
for r=0:1:rmax     %�� for ѭ������������г���������ڣ�ֱ�����һ end �Ž���ѭ��
    r
  %ÿ��һ����ת����(������Ϊ10��)ʹ���ڵ��S��i��.G�������ò������ں���Ĵ�ѡ�٣��ڸ���ת�������ѵ�ѡ����ͷ�Ľڵ㲻���ٵ�ѡ���ָ�Ϊ��
  if(mod(r, round(1/P) )==0)
    for i=1:1:n
        S3(i).G=0;
        S3(i).cl=0;
    end
  end
  Ea=Et*(1-r/rmax)/n;
  El3(r+1)=0;
  for i=1:100
    El3(r+1)=S3(i).E+El3(r+1);
  end
Ec3(r+1)=Et-El3(r+1);
%(2)�����ڵ���ģ��
dead3=0;
for i=1:1:n
    %������������ڵ�
    if (S3(i).E<=0)
        dead3=dead3+1; 
        %(3)��һ�������ڵ�Ĳ���ʱ��(���ִα�ʾ)
        %��һ���ڵ�����ʱ��
        if (dead3==1)
           if(flag_first_dead3==0)
              first_dead3=r;
              flag_first_dead3=1;
           end
        end
        %10%�Ľڵ�����ʱ��
        if(dead3==0.1*n)
           if(flag_teenth_dead3==0)
              teenth_dead3=r;
              flag_teenth_dead3=1;
           end
        end
        if(dead3==n)
           if(flag_all_dead3==0)
              all_dead3=r;
              flag_all_dead3=1;
           end
        end
    end
    if S3(i).E>0
        S3(i).type='N';
    end
end
STATISTICS.DEAD3(r+1)=dead3;
STATISTICS.ALLIVE3(r+1)=allive3-dead3;
%(4)��ͷѡ��ģ��
countCHs3=0;
cluster3=1;
for i=1:1:n
 if Ea>0
 p(i)=P*n*S3(i).E*E3(i)/(Et*Ea);
 if(S3(i).E>0)
   temp_rand=rand;     
   if ( (S3(i).G)<=0)  
       %��ͷ��ѡ�٣���ѡ�Ĵ�ͷ��Ѹ�������Ŵ�����������������ı�����
        if(temp_rand<= (p(i)/(1-p(i)*mod(r,round(1/p(i))))))
            countCHs3=countCHs3+1;
            packets_TO_BS3=packets_TO_BS3+1;
            PACKETS_TO_BS3(r+1)=packets_TO_BS3;
            S3(i).type='C';
            S3(i).G=round(1/p(i))-1;
            C3(cluster3).xd=S3(i).xd;
            C3(cluster3).yd=S3(i).yd;
            distance=sqrt( (S3(i).xd-(S3(n+1).xd) )^2 + (S3(i).yd-(S3(n+1).yd) )^2 );
            C3(cluster3).distance=distance;
            C3(cluster3).id=i;
            X3(cluster3)=S3(i).xd;
            Y3(cluster3)=S3(i).yd;
            cluster3=cluster3+1;
            %�����ͷ����4000bit���ݵ���վ���������ģ�����Ӧ�����нڵ������ͷÿһ�ַ���4000bit���ݣ�
            distance;
            if (distance>do)
                S3(i).E=S3(i).E- ( (ETX+EDA)*(4000) + Emp*4000*( distance*distance*distance*distance )); 
            end
            if (distance<=do)
                S3(i).E=S3(i).E- ( (ETX+EDA)*(4000)  + Efs*4000*( distance * distance )); 
            end
        end     
    
    end
    % S3(i).G=S3(i).G-1;  
   
  end 
 end
end
STATISTICS.COUNTCHS3(r+1)=countCHs3;
%(5)���ڳ�Աѡ���ͷģ��(���ص��γ�ģ��)
%���ڳ�Ա�Դ�ͷ��ѡ�񣨼��ص��γɣ��㷨
for c=1:1:cluster3-1
    x3(c)=0;
end
y3=0;
z3=0;
for i=1:1:n
   if ( S3(i).type=='N' && S3(i).E>0 )
     if(cluster3-1>=1)
       min_dis=Inf;
       min_dis_cluster=0;
       for c=1:1:cluster3-1
           temp=min(min_dis,sqrt( (S3(i).xd-C3(c).xd)^2 + (S3(i).yd-C3(c).yd)^2 ) );
           if ( temp<min_dis )
               min_dis=temp;
               min_dis_cluster=c;
               x3(c)=x3(c)+1;
           end
       end
       %���ڽڵ㣨����4000bit���ݣ���������
 
            min_dis;
            if (min_dis>do)
                S3(i).E=S3(i).E- ( ETX*(4000) + Emp*4000*( min_dis * min_dis * min_dis * min_dis)); 
            end
            if (min_dis<=do)
                S3(i).E=S3(i).E- ( ETX*(4000) + Efs*4000*( min_dis * min_dis)); 
            end
        %��ͷ�����ܺ��ں���һ���ڽڵ�4000bit���ݣ�����������
            S3(C3(min_dis_cluster).id).E = S3(C3(min_dis_cluster).id).E- ( (ERX + EDA)*4000 ); 
            packets_TO_CH3=packets_TO_CH3+1;

            
      
        S3(i).min_dis=min_dis;
        S3(i).min_dis_cluster=min_dis_cluster;
   else
            y3=y3+1;
            min_dis=sqrt( (S3(i).xd-S3(n+1).xd)^2 + (S3(i).yd-S3(n+1).yd)^2 );
            if (min_dis>do)
                S3(i).E=S3(i).E- ( ETX*(4000) + Emp*4000*( min_dis * min_dis * min_dis * min_dis)); 
            end
            if (min_dis<=do)
                S3(i).E=S3(i).E- ( ETX*(4000) + Efs*4000*( min_dis * min_dis)); 
            end
            packets_TO_BS3=packets_TO_BS3+1;
   end
  end
end
if countCHs3~=0
   u3=(n-y3)/countCHs3;
 for c=1:1:cluster3-1
    z3=(x3(c)-u3)*(x3(c)-u3)+z3;
 end
 LBF3(r+1)=z3/countCHs3;
else  LBF3(r+1)=0;
end
 STATISTICS.PACKETS_TO_CH3(r+1)=packets_TO_CH3;
 STATISTICS.PACKETS_TO_BS3(r+1)=packets_TO_BS3;
end
countCHs4=0;
cluster=1;%�˶����Ŀ�Ľ����Ǹ���һ��1��ʼ���±�����������Ĵ�ͷ��Ӧ�û���ȥ1
flag_first_dead4=0;
flag_teenth_dead4=0;
flag_all_dead4=0;
%�����ڵ���
dead4=0;
first_dead4=0;
teenth_dead4=0;
all_dead4=0;
%��ڵ���
allive4=n;
%counter for bit transmitted to Bases Station and to Cluster Heads
packets_TO_BS4=0;
packets_TO_CH4=0;
%(1)ѭ��ģʽ�趨
for r=0:1:rmax     %�� for ѭ������������г���������ڣ�ֱ�����һ end �Ž���ѭ��
    r
  %ÿ��һ����ת����(������Ϊ10��)ʹ���ڵ��S��i��.G�������ò������ں���Ĵ�ѡ�٣��ڸ���ת�������ѵ�ѡ����ͷ�Ľڵ㲻���ٵ�ѡ���ָ�Ϊ��
  if(mod(r, round(1/P) )==0)
    for i=1:1:n
        S4(i).G=0;
        S4(i).cl=0;
    end
  end
  Ea=Et*(1-r/rmax)/n;
  El4(r+1)=0;
  for i=1:100
   El4(r+1)=S4(i).E+El4(r+1);
  end
  Ec4(r+1)=Et-El4(r+1);
%(2)�����ڵ���ģ��
dead4=0;
for i=1:1:n
    %������������ڵ�
    if (S4(i).E<=0)
        dead4=dead4+1; 
        %(3)��һ�������ڵ�Ĳ���ʱ��(���ִα�ʾ)
        %��һ���ڵ�����ʱ��
        if (dead4==1)
           if(flag_first_dead4==0)
              first_dead4=r;
              flag_first_dead4=1;
           end
        end
        %10%�Ľڵ�����ʱ��
        if(dead4==0.1*n)
           if(flag_teenth_dead4==0)
              teenth_dead4=r;
              flag_teenth_dead4=1;
           end
        end
        if(dead4==n)
           if(flag_all_dead4==0)
              all_dead4=r;
              flag_all_dead4=1;
           end
        end
    end
    if S4(i).E>0
        S4(i).type='N';
    end
end
STATISTICS.DEAD4(r+1)=dead4;
STATISTICS.ALLIVE4(r+1)=allive4-dead4;
%(4)��ͷѡ��ģ��
countCHs4=0;
cluster4=1;
for i=1:1:n
 if Ea>0
 p(i)=P*n*S4(i).E*E4(i)/(Et*Ea);
 if(S4(i).E>0)
   temp_rand=rand;     
   if ( (S4(i).G)<=0)  
       %��ͷ��ѡ�٣���ѡ�Ĵ�ͷ��Ѹ�������Ŵ�����������������ı�����
        if(temp_rand<= (p(i)/(1-p(i)*mod(r,round(1/p(i))))))
            countCHs4=countCHs4+1;
            packets_TO_BS4=packets_TO_BS4+1;
            PACKETS_TO_BS4(r+1)=packets_TO_BS4;
            S4(i).type='C';
            S4(i).G=round(1/p(i))-1;
            C4(cluster4).xd=S4(i).xd;
            C4(cluster4).yd=S4(i).yd;
            distance=sqrt( (S4(i).xd-(S4(n+1).xd) )^2 + (S4(i).yd-(S4(n+1).yd) )^2 );
            C4(cluster4).distance=distance;
            C4(cluster4).id=i;
            X4(cluster4)=S4(i).xd;
            Y4(cluster4)=S4(i).yd;
            cluster4=cluster4+1;
           %�����ͷ����4000bit���ݵ���վ���������ģ�����Ӧ�����нڵ������ͷÿһ�ַ���4000bit���ݣ�
            distance;
            if (distance>do)
                S4(i).E=S4(i).E- ( (ETX+EDA)*(4000) + Emp*4000*( distance*distance*distance*distance )); 
            end
            if (distance<=do)
                S4(i).E=S4(i).E- ( (ETX+EDA)*(4000)  + Efs*4000*( distance * distance )); 
            end
        end     
    
    end
    % S4(i).G=S4(i).G-1;  
  end 
 end
end
STATISTICS.COUNTCHS4(r+1)=countCHs4;
%(5)���ڳ�Աѡ���ͷģ��(���ص��γ�ģ��)
%���ڳ�Ա�Դ�ͷ��ѡ�񣨼��ص��γɣ��㷨
for c=1:1:cluster4-1
    x4(c)=0;
end
y4=0;
z4=0;
for i=1:1:n
   if ( S4(i).type=='N' && S4(i).E>0 )
     if(cluster4-1>=1)
       min_dis=sqrt( (S4(i).xd-S4(n+1).xd)^2 + (S4(i).yd-S4(n+1).yd)^2 );
       min_dis_cluster=0;
       for c=1:1:cluster4-1
           temp=min(min_dis,sqrt( (S4(i).xd-C4(c).xd)^2 + (S4(i).yd-C4(c).yd)^2 ) );
           if ( temp<min_dis )
               min_dis=temp;
               min_dis_cluster=c;
               x4(c)=x4(c)+1;
           end
       end
       %���ڽڵ㣨����4000bit���ݣ���������
       if(min_dis_cluster~=0)    
            min_dis;
            if (min_dis>do)
                S4(i).E=S4(i).E- ( ETX*(4000) + Emp*4000*( min_dis * min_dis * min_dis * min_dis)); 
            end
            if (min_dis<=do)
                S4(i).E=S4(i).E- ( ETX*(4000) + Efs*4000*( min_dis * min_dis)); 
            end
        %��ͷ�����ܺ��ں���һ���ڽڵ�4000bit���ݣ�����������
            S4(C4(min_dis_cluster).id).E = S4(C4(min_dis_cluster).id).E- ( (ERX + EDA)*4000 ); 
            packets_TO_CH4=packets_TO_CH4+1;
       else 
            min_dis;
            if (min_dis>do)
                S4(i).E=S4(i).E- ( ETX*(4000) + Emp*4000*( min_dis * min_dis * min_dis * min_dis)); 
            end
            if (min_dis<=do)
                S4(i).E=S4(i).E- ( ETX*(4000) + Efs*4000*( min_dis * min_dis)); 
            end
            packets_TO_BS4=packets_TO_BS4+1;
            
       end
       S4(i).min_dis=min_dis;
       S4(i).min_dis_cluster=min_dis_cluster;
   else
            y4=y4+1;
            min_dis=sqrt( (S4(i).xd-S4(n+1).xd)^2 + (S4(i).yd-S4(n+1).yd)^2 );
            if (min_dis>do)
                S4(i).E=S4(i).E- ( ETX*(4000) + Emp*4000*( min_dis * min_dis * min_dis * min_dis)); 
            end
            if (min_dis<=do)
                S4(i).E=S4(i).E- ( ETX*(4000) + Efs*4000*( min_dis * min_dis)); 
            end
            packets_TO_BS4=packets_TO_BS4+1;
   end
  end
end
if countCHs4~=0
   u4=(n-y4)/countCHs4;
 for c=1:1:cluster4-1
    z4=(x4(c)-u4)*(x4(c)-u4)+z4;
 end
 LBF4(r+1)=z4/countCHs4;
 else  LBF4(r+1)=0;
 end
STATISTICS.PACKETS_TO_CH4(r+1)=packets_TO_CH4;
STATISTICS.PACKETS_TO_BS4(r+1)=packets_TO_BS4;
end
r=0:5000;
plot(r,STATISTICS.DEAD3,':b',r,STATISTICS.DEAD4,'-r');
legend('DEEC1','DEEC2');
xlabel('x(time)');
ylabel('y(dead)');
title('\bf DEEC1��DEEC2�������ڵ�����ʱ��ı仯�Ա�');
%plot(r,STATISTICS.PACKETS_TO_BS3,':b',r,STATISTICS.PACKETS_TO_BS4,'-r');
%legend('DEEC1','DEEC2');
%xlabel('x(time)');
%ylabel('y(data)');
%title('\bf DEEC1��DEEC2�����ݴ���Ա�');
%r=0:5000;
%plot(r,Ec3,':b',r,Ec4,'-r');
%legend('DEEC1','DEEC2');
%xlabel('x(time)');
%ylabel('y(consumption)');
%title('\bf DEEC1��DEEC2�������������ĶԱ�');
%r=0:100:5000;
%plot(r,STATISTICS.COUNTCHS3(r+1),':b',r,STATISTICS.COUNTCHS4(r+1),'-r');
%legend('DEEC1','DEEC2');
%xlabel('x(time)');
%ylabel('y(clusters)');
%title('\bf DEEC1��DEEC2�Ĵ�ͷ�������Ա�');
%r=0:100:5000;
%plot(r,LBF3(r+1),':b',r,LBF4(r+1),'-r');
%legend('DEEC1','DEEC2');
%xlabel('x(time)');
%ylabel('y(balance)');
%title('\bf DEEC1��DEEC2�ĸ��ؾ���ȶԱ�');