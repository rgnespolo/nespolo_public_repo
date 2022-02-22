"""Rogerio Garcia Nespolo
Created on March 2021
Goal: To predict the log of PSA (lpsa) related to a number of clinical measures present in the dataset
Why? The higher the PSA level, the higher the chances of prostate cancer.
"""
import pandas as pd
from sklearn import linear_model
import numpy as np
from matplotlib import pyplot as plt

#read the .data, separating by space
df = pd.read_csv('https://hastie.su.domains/ElemStatLearn/datasets/prostate.data', sep='\t')
#shuffle the original data
df.reindex(np.random.permutation(df.index))
#need to randomize here, own implementation

df = df.iloc[:,:-1]
#df = df.drop('train', axis = 1)
df = df.drop(df.index[0])

#normalize the data, except the lpsa
df_normalized = (df.iloc[:, 1:-1] - df.iloc[:, 1:-1].min()) / (df.iloc[:, 1:-1].max() - df.iloc[:, 1:-1].min())
#Return lpsa to dataframe
dataset =  pd.concat([df_normalized, df.iloc[:, 1:-1]], axis=1)

#global parameters
k = 5
folds = np.array_split(dataset, k)
# the eta for the grad. desc. was chosen according to the performance measured
# by the r2 score. With eta = 0.001 for example, the r2 score is worse than a 
# higher eta. In this case, the model accepted a higher eta without overshooting 
eta = 0.0001

# the number of iterations was set observing the convergence of the gradient 
# descent to a satisfying small error.
iterations = 10000
# define lasso model - alpha was choosen considering the r2 score of the model
# it is observed, in this case, that smaller the alpha, better the  r2 score for
# the lasso regression. With alpha = 0.001, the r2 score was 0.9999 (possible overfitting?)
lasso_reg = linear_model.Lasso(alpha=0.03)


def main():
    
    #create the folds for k-fold validation
    for i in range(k):
        #make a copy of all the folds
        train = folds.copy() 
        #select a fold to test
        test = folds[i]
        # delete the fold selected to test 
        del train[i]
        #put 4 folds together, leaving the deleted one as test
        train = pd.concat(train, sort=False)
        x_train = (train.iloc[:, :7]).to_numpy() #features
        x_test = (test.iloc[:, :7]).to_numpy() #features
        y_train = (train.iloc[:, 8]).to_numpy() #output
        y_test = (test.iloc[:, 8]).to_numpy() #output
        
#------------------gradient descent regression -----------------------------------------
        W = np.zeros(x_train.shape[1])
        new_W, error_history = batch_gradient_descent(x_train, y_train, W, eta, iterations)
        y_pred = pred(x_test, new_W)
        score = r2(y_pred,y_test)
        
        print('Grad. descent r2 score for epoch %d: %s ' %(i+1, score))
        plt.figure()
        plt.scatter(y_pred, y_test)
        plt.title('gradient desc. predictions for the fold {} '.format(i+1))
        plt.xlabel('test values')
        plt.ylabel('Predictions')
        #convert error to dataframe to make things easier to plot
        error = pd.DataFrame({'error':error_history})
        plt.figure()
        plt.scatter(error.index.values, error['error'])
        plt.title('Gradient desc. convergence for the fold {} '.format(i+1))
        plt.xlabel('epoch')
        plt.ylabel('error')
        
 #----------------- lasso regression ----------------------------------------------------
        lasso_reg.fit(x_train, y_train)
        pred_lasso = lasso_reg.predict(x_test)
        #print(pred_kfold)
        plt.figure()
        plt.scatter(y_test, pred_lasso, color = 'red')
        plt.title('Lasso predictions for the fold {} '.format(i+1))
        plt.xlabel('true values')
        plt.ylabel('lasso predictions')
        score_lasso = r2(pred_lasso,y_test)
        print('Lasso r2 score for epoch %d: %s ' %(i+1, score_lasso))

def error_function(X, Y, W): #mean square error
    m = len(Y)
    J = np.sum((X.dot(W)- Y) ** 2) / (2 * m)
    return J

def batch_gradient_descent(X, Y, W, eta, iterations):
    #reset the error and allocate the array to store all the errors for each iteration
    error_history = [0] * iterations
    m = len(Y)

    for iteration in range(iterations):
        h = X.dot(W)
        loss = h - Y # Gradient Calculation
        gradient = X.T.dot(loss) / m
        W = W- eta * gradient #calculate new weight
        error = error_function(X, Y, W)
        error_history[iteration] = error # store the error after each new weight    
    return W, error_history

#prediction function -> Cost
def pred(x_test, new_W):
    return x_test.dot(new_W)


# calculate the r2 score
def r2(y_pred,y):
    # sum of square totals
    sst = np.sum((y-y.mean())**2)
    #sum of square of expected values
    ssr = np.sum((y_pred-y)**2)
    r2 = 1-(ssr/sst)
    return(r2)

main()


