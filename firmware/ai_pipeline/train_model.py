import pandas as pd
from sklearn.tree import DecisionTreeClassifier, export_text

# Đọc dữ liệu
df = pd.read_csv('car_dataset.csv').dropna()
X = df[['pitch', 'roll']]
y = df['label']

# Huấn luyện AI với max_depth=3 để code C không bị quá dài
model = DecisionTreeClassifier(max_depth=4, random_state=42)
model.fit(X, y)

# In ra cấu trúc AI để copy vào STM32
print("\n--- CAU TRUC CAY QUYET DINH (COPY VAO STM32) ---")
tree_rules = export_text(model, feature_names=['pitch', 'roll'])
print(tree_rules.replace("<=", "<=").replace("class: 0", "DUNG XE").replace("class: 1", "TOI").replace("class: 2", "LUI").replace("class: 3", "PHAI").replace("class: 4", "TRAI"))