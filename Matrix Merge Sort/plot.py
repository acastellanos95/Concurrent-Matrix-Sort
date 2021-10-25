import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt
sns.set_theme(style="whitegrid")

data = pd.read_csv("./matriz_a.data", " ", header=0)
print(data)

data = data[['THREADS', 'MATRIX_SIZE', 'REAL_TIME']]
Matrix_Size_Order = data['MATRIX_SIZE']
data_wide = data.pivot_table(index='THREADS', columns='MATRIX_SIZE', values=['REAL_TIME'])
# data_wide = data_wide.reindex(Matrix_Size_Order, axis=0)
print(data_wide)

plot = sns.lineplot(data=data_wide['REAL_TIME'])
plt.savefig("time_20000.png")
