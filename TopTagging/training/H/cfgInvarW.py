#!/usr/bin/env python


variables = [ (x,'F') for x in 
      [
        'tau21SD',
        'alphapull1',
        'alphapull2',
        'alphapull3',
        'qtau21'
      ]
]

formulae = [ (x,'F') for x in 
    [   
"ecfN_2_4_10/pow(ecfN_1_2_05,4.00)" ,
 "ecfN_2_3_05/pow(ecfN_1_2_05,2.00)", 
 "ecfN_2_3_10/pow(ecfN_1_2_10,2.00)", 
 "ecfN_2_4_20/pow(ecfN_1_2_10,4.00)", 
 "ecfN_1_4_20/pow(ecfN_1_2_05,4.00)",
 "ecfN_3_3_10/pow(ecfN_1_2_10,3.00)", 
 "ecfN_3_3_20/pow(ecfN_1_2_20,3.00)", 
 "ecfN_2_3_20/pow(ecfN_1_2_20,2.00)", 
 "ecfN_2_4_10/pow(ecfN_2_3_05,2.00)", 
 "ecfN_3_3_05/pow(ecfN_1_2_10,1.50)", 
 "ecfN_1_3_20/pow(ecfN_1_2_10,2.00)", 
 "ecfN_2_3_20/pow(ecfN_1_2_10,4.00)", 
 "ecfN_1_4_20/pow(ecfN_2_3_05,2.00)", 
 "ecfN_1_3_10/pow(ecfN_1_2_05,2.00)", 
 "ecfN_3_3_10/pow(ecfN_1_2_20,1.50)",
 "ecfN_3_3_05/pow(ecfN_1_2_05,3.00)", 
 "ecfN_2_4_20/pow(ecfN_2_3_10,2.00)", 
 "ecfN_1_3_20/pow(ecfN_1_2_05,4.00)", 
 "ecfN_2_4_20/pow(ecfN_1_2_20,2.00)", 
 "ecfN_2_4_10/pow(ecfN_1_2_10,2.00)", 
 "ecfN_1_4_20/pow(ecfN_1_3_10,2.00)", 
 "ecfN_2_3_10/pow(ecfN_1_2_05,4.00)", 
 "ecfN_1_4_20/pow(ecfN_1_2_10,2.00)", 
 "ecfN_2_4_05/pow(ecfN_1_2_05,2.00)", 
 "ecfN_2_4_20/pow(ecfN_3_3_05,2.67)", 
 "ecfN_1_4_10/pow(ecfN_1_3_05,2.00)", 
 "ecfN_2_4_20/pow(ecfN_3_3_10,1.33)", 
 "ecfN_1_4_10/pow(ecfN_1_2_05,2.00)", 
 "ecfN_2_4_10/pow(ecfN_3_3_05,1.33)", 
 "ecfN_1_4_20/pow(ecfN_3_3_05,1.33)", 
 "ecfN_1_4_05/ecfN_1_2_05", 
 "ecfN_1_3_05/ecfN_1_2_05",
 "ecfN_2_3_05/ecfN_1_2_10",
 "ecfN_2_4_05/ecfN_1_2_10",
 "ecfN_2_4_05/ecfN_2_3_05",
 "ecfN_1_4_10/ecfN_1_2_10",
 "ecfN_2_4_05/ecfN_1_3_10",
 "ecfN_1_4_10/ecfN_2_3_05",
 "ecfN_1_4_05/pow(ecfN_1_2_10,0.50)",
 "ecfN_1_4_05/ecfN_1_3_05",
 "ecfN_1_3_10/ecfN_1_2_10",
 "ecfN_1_4_05/pow(ecfN_2_3_05,0.50)",
 "ecfN_1_4_20/ecfN_1_2_20",
 "ecfN_2_4_10/ecfN_2_3_10",
 "ecfN_2_4_10/ecfN_1_2_20",
 "ecfN_1_3_05/pow(ecfN_1_2_10,0.50)",
 "ecfN_1_4_20/ecfN_2_3_10",
 "ecfN_1_4_10/ecfN_1_3_10",
 "ecfN_1_4_05/pow(ecfN_3_3_05,0.33)",
 "ecfN_2_3_10/ecfN_1_2_20",
    ]
]

spectators = [
          ('tau21','F'),
          ('tau21SD','F'),
          ('pt','F'),
          ('mSD','F'),
              ]
