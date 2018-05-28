import matplotlib
import os
from matplotlib import rcParams
rcParams['font.family'] = 'serif'
rcParams['font.sans-serif'] = ['Palatino']
import matplotlib.pyplot as plt
import numpy as np

def plot_comparison(path, title, outfile):
    data = np.genfromtxt(path, delimiter=',', names=['x_qh', 'y_qh', 'x_inc', 'y_inc'])

    fig, ax = plt.subplots()

    ax.plot(data['x_qh'], data['y_qh'], color='black', label='QuickHull', marker='x', markersize=7.0)
    ax.plot(data['x_inc'], data['y_inc'], color='black', label='Incremental', marker='^', markersize=7.0)
    ax.get_xaxis().set_major_formatter(
    matplotlib.ticker.FuncFormatter(lambda x, p: format(int(x), ',')))
    ax.get_yaxis().set_major_formatter
    (
        matplotlib.ticker.FuncFormatter(lambda x, p: format(round(float(x)/1000000.0,2)))
    )

    plt.xlabel('Number of vertices')
    plt.ylabel('Time spent')
    plt.title(title)
    savefig(outfile)
    plt.show()

def plot_nlogn(path, title, outfile, miny = [0.0, 0.5], minrange=0):
    data = np.genfromtxt(path, delimiter=',', names=['x_qh', 'y_qh', 'x_inc', 'y_inc'])

    fig, ax = plt.subplots()

    data['y_qh'] = [np.divide(y, np.multiply(x, np.log2(x))) for y,x in zip(data['y_qh'], data['x_qh'])]
    data['y_inc'] = [np.divide(y, np.multiply(x, np.log2(x))) for y,x in zip(data['y_inc'], data['x_inc'])]
    ax.plot(data['x_qh'][minrange:,], data['y_qh'][minrange:,], color='black', label='QuickHull', marker='x', markersize=7.0)
    ax.plot(data['x_inc'][minrange:,], data['y_inc'][minrange:,], color='black', label='Incremental', marker='^', markersize=7.0)
    #ax.plot(data['x_inc'], data['y_inc'], color='black', label='data', marker='^', markersize=7.0)
    ax.get_xaxis().set_major_formatter(
    matplotlib.ticker.FuncFormatter(lambda x, p: format(int(x), ',')))
    ax.get_yaxis().set_major_formatter
    (
        matplotlib.ticker.FuncFormatter(lambda x, p: format(round(float(x)/1000000.0,2)))
    )
    ax.set_ylim(miny)
    ax.legend()

    plt.xlabel('Number of vertices')
    plt.ylabel('Time spent over n log n')
    plt.title(title)
    savefig(outfile)
    plt.show()

def plot_nsquared(path, title, outfile, miny = [0.0, 0.5], minrange=0):
    data = np.genfromtxt(path, delimiter=',', names=['x_qh', 'y_qh', 'x_inc', 'y_inc'])

    fig, ax = plt.subplots()

    data['y_qh'] = [np.divide(y, np.multiply(x, x)) for y,x in zip(data['y_qh'], data['x_qh'])]
    data['y_inc'] = [np.divide(y, np.multiply(x, x)) for y,x in zip(data['y_inc'], data['x_inc'])]
    ax.plot(data['x_qh'][minrange:,], data['y_qh'][minrange:,], color='black', label='QuickHull', marker='x', markersize=7.0)
    ax.plot(data['x_inc'][minrange:,], data['y_inc'][minrange:,], color='black', label='Incremental', marker='^', markersize=7.0)
    #ax.plot(data['x_inc'], data['y_inc'], color='black', label='data', marker='^', markersize=7.0)
    ax.get_xaxis().set_major_formatter(
    matplotlib.ticker.FuncFormatter(lambda x, p: format(int(x), ',')))
    ax.get_yaxis().set_major_formatter
    (
        matplotlib.ticker.FuncFormatter(lambda x, p: format(round(float(x)/1000000.0,2)))
    )
    ax.set_ylim(miny)
    ax.legend()

    plt.xlabel('Number of vertices')
    plt.ylabel('Time spent over n log n')
    plt.title(title)
    savefig(outfile)
    plt.show()

def savefig(path):
    plt.savefig(os.path.join('graphs' + os.sep, path + '.pdf'), format='pdf', dpi=1000)
    

if __name__ == '__main__':
    plot_comparison('c:\\Users\\Niels\\projects\\Thesis-Convex\\data\\in_sphere_10m.csv', 'Time for points in a sphere', 'time_qh_inc_in_sphere')
    plot_nlogn('c:\\Users\\Niels\\projects\\Thesis-Convex\\data\\in_sphere_10m.csv', 'Time for points in a sphere over nlogn', 'time_qh_inc_in_sphere_nlogn', minrange=15)
    plot_comparison('c:\\Users\\Niels\\projects\\Thesis-Convex\\data\\on_sphere_100k.csv', 'Time for points on a sphere', 'time_qh_inc_on_sphere')
    plot_nlogn('c:\\Users\\Niels\\projects\\Thesis-Convex\\data\\on_sphere_100k.csv', 'Time for points on a sphere over nlogn', 'time_qh_inc_on_sphere_nlogn', [0.0,10.5])
    plot_nsquared('c:\\Users\\Niels\\projects\\Thesis-Convex\\data\\on_sphere_100k.csv', 'Time for points on a sphere over nsquared', 'time_qh_inc_on_sphere_squard', [0.0,0.01], 5)