import math

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from imblearn.over_sampling import SMOTE
from m2cgen import export_to_c
from sklearn import tree
from sklearn.feature_selection import SelectPercentile
from sklearn.feature_selection import mutual_info_regression
from sklearn.metrics import classification_report
from sklearn.model_selection import GridSearchCV
from sklearn.model_selection import cross_validate
from sklearn.pipeline import Pipeline


def cross_val(regressor, X, y, k=10):
    s = ['neg_mean_absolute_error', 'neg_root_mean_squared_error', 'r2']
    results = cross_validate(regressor, X, y, cv=k, scoring=s)
    print(pd.DataFrame(results).mean())


def plot_results(regressor, X_train, y_train, X_test, y_test, y):
    model = regressor.fit(X_train, y_train)
    y_pred = model.predict(X_test)

    errors = np.abs(y_pred - y_test)
    errors = errors / max(errors)

    plt.figure(figsize=(8, 6))
    plt.scatter(y_test, y_pred, s=200 * errors, c=errors, alpha=0.7, marker='x')
    plt.ylim(0, max(y_pred.max(), y_test.max()))
    plt.xlim(0, max(y_pred.max(), y_test.max()))
    plt.xlabel('Value')
    plt.ylabel('Predicted')

    plt.plot([0, max(y)], [0, max(y)], c='gray')


def correlation(dataset, threshold):
    col_corr = set()
    corr_matrix = dataset.corr()
    for i in range(len(corr_matrix.columns)):
        for j in range(i):
            if abs(corr_matrix.iloc[i, j]) > threshold:
                if (corr_matrix.columns[i] not in col_corr) and (corr_matrix.index.tolist()[j] not in col_corr):
                    col_name = corr_matrix.columns[i]
                    col_corr.add(col_name)
    return col_corr


def grid_search(model, g_df, s, features, params):
    seq_names = g_df['name'].drop_duplicates().sample(frac=1)

    size_slice = math.floor(len(seq_names) * s)

    seq_test = seq_names[0: size_slice]
    seq_train = pd.concat([seq_names[size_slice:]])

    df_k_test = g_df[g_df.name.isin(seq_test)]
    df_k_train = g_df[g_df.name.isin(seq_train)]

    df_k_X_test = df_k_test.drop(['name', 'selected'], axis=1)
    df_k_y_test = df_k_test['selected']
    df_k_X_train = df_k_train.drop(['name', 'selected'], axis=1)
    df_k_y_train = df_k_train['selected']

    df_k_X_test = df_k_X_test[features]
    df_k_X_train = df_k_X_train[features]

    smote = SMOTE()
    df_k_X_train, df_k_y_train = smote.fit_resample(df_k_X_train, df_k_y_train)

    pipe = Pipeline(steps=[('model', model)])

    clf_GS = GridSearchCV(pipe, params, cv=5)
    clf_GS.fit(df_k_X_train, df_k_y_train)

    model.fit(df_k_X_train, df_k_y_train)
    df_k_y_pred = clf_GS.best_estimator_.predict(df_k_X_test)

    return clf_GS, classification_report(df_k_y_test, df_k_y_pred, output_dict=False)


df = pd.read_csv("dataset.csv", low_memory=False)

seq_test = [
    'BQSquare_416x240_60',
    'Flowervase_416x240_30',
    'tempete_cif',
    'waterfall_cif',
    'PartyScene_832x480_50',
    'aspen_480p_30f',
    'aspen_1080p',
    'into_tree_480p_30f',
    'in_to_tree_420_720p50',
    'in_to_tree_1080p50',
    'station2_480p25',
    'station2_1080p25'
    'blue_sky_1080p25',
    'BlueSky_360p25',
    'Netflix_DrivingPOV_1280x720_60fps_8bit_420_120f',
    'mobcal_720p_30f',
]

seq_drop = [
    'crowd_run_1080p50'
]

df = df[~df.name.isin(seq_drop)]

df.drop([
    'cur_ref_count',
    'cur_showable_frame',
    'ref_showable_frame',
    'downsample_level',
    'num_refinements',
    'bit_depth',
    'warp_error'
], axis=1, inplace=True)

data_test = df[df.name.isin(seq_test)].copy()
data_train = df[~df.name.isin(seq_test)].copy()

X = data_train.drop(['selected'], axis=1)
y = data_train['selected']

corr_features = correlation(X.drop('name', axis=1), 0.8)
corr_features = corr_features.difference(['fast_error', 'ref_frame_error'])

X = X.drop(corr_features, axis=1)

selected_top_columns = SelectPercentile(mutual_info_regression, percentile=50)
selected_top_columns.fit(X.drop('name', axis=1), y)

select_features = X.drop('name', axis=1).columns[selected_top_columns.get_support()]

m_data = data_train.copy()

dtc = tree.DecisionTreeClassifier()

criterion = ['gini', 'entropy']
max_depth = [2, 4, 6, 8, 10, 12, 14]
min_samples_split = [2, 3]
min_samples_leaf = [1, 2, 3]

params = dict(model__criterion=criterion,
              model__max_depth=max_depth,
              model__min_samples_split=min_samples_split,
              model__min_samples_leaf=min_samples_leaf)

clf_GS, report = grid_search(dtc, m_data, 0.3, select_features, params)

print(report)
print()
print(clf_GS.best_estimator_.get_params()['model'])

model = tree.DecisionTreeClassifier(max_depth=8)

X_train = data_train[select_features]
y_train = data_train['selected']

X_test = data_test[select_features]
y_test = data_test['selected']

smote = SMOTE()
X_train, y_train = smote.fit_resample(X_train, y_train)

model.fit(X_train, y_train)
y_pred = model.predict(X_test)

with open('model.c', 'w') as file:
    file.write(export_to_c(model=model))

print(classification_report(y_test, y_pred))
