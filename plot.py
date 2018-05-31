import matplotlib
import os
from matplotlib import rcParams
rcParams['font.family'] = 'serif'
rcParams['font.sans-serif'] = ['Palatino']
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.ticker import FormatStrFormatter

def format_num():
    return (lambda x, p: x)
    
def format_axis(ax):
    ax.get_xaxis().set_major_formatter(
        matplotlib.ticker.FuncFormatter(lambda x, p: format(int(x), ','))
    )

    ax.yaxis.set_major_formatter
    (
        matplotlib.ticker.FuncFormatter(format_num())
    )

def plot_single(path, title, outfile, label, ylabel):
    data = np.genfromtxt(path, delimiter=',', names=['x', 'y'])

    data['y'] = [np.divide(y, 1000000.0) for y in data['y']]

    fig, ax = plt.subplots()
    ax.plot(data['x'], data['y'], color='black', label=label, marker='x', markersize=7.0)
    format_axis()
    ax.legend()

    plt.xlabel('Number of vertices')
    plt.ylabel(ylabel)
    plt.title(title)
    savefig(outfile)
    plt.show()


def plot_comparison(path, title, outfile, ylabel):
    data = np.genfromtxt(path, delimiter=',', names=['x_qh', 'y_qh', 'x_inc', 'y_inc'])

    fig, ax = plt.subplots()

    data['y_qh'] = [np.divide(y, 1000000.0) for y in data['y_qh']]
    data['y_inc'] = [np.divide(y, 1000000.0) for y in data['y_inc']]

    ax.plot(data['x_qh'], data['y_qh'], color='black', label='QuickHull', marker='x', markersize=7.0)
    ax.plot(data['x_inc'], data['y_inc'], color='black', label='Incremental', marker='^', markersize=7.0)
    format_axis(ax)
    
    ax.legend()

    plt.xlabel('Number of vertices')
    plt.ylabel(ylabel)
    plt.title(title)
    savefig(outfile)
    plt.show()

def plot_three(path, title, outfile, ylabel):
    data = np.genfromtxt(path, delimiter=',', names=['x_qh', 'y_qh', 'x_inc', 'y_inc', 'x_out', 'y_out'])

    fig, ax = plt.subplots()

    ax.plot(data['x_qh'], data['y_qh'], color='black', label='QuickHull', marker='x', markersize=6.0)
    ax.plot(data['x_inc'], data['y_inc'], color='black', label='Incremental', marker='^', markersize=6.0)
    ax.plot(data['x_out'], data['y_out'], color='black', label='Points on hull', marker='p', markersize=6.0)
    format_axis(ax)
    
    ax.legend()

    plt.xlabel('Number of vertices')
    plt.ylabel(ylabel)
    plt.title(title)
    savefig(outfile)
    plt.show()

def plot_single_nlogn(path, title, outfile, label, miny = [0.0, 0.5], minrange=0):
    data = np.genfromtxt(path, delimiter=',', names=['x', 'y'])

    fig, ax = plt.subplots()

    data['y'] = [np.divide(y, np.multiply(x, np.log2(x))) for y,x in zip(data['y'], data['x'])]
    ax.plot(data['x'][minrange:,], data['y'][minrange:,], color='black', label=label, marker='x', markersize=7.0)
    #ax.plot(data['x_inc'], data['y_inc'], color='black', label='data', marker='^', markersize=7.0)
    format_axis()

    ax.set_ylim(miny)
    ax.legend()

    plt.xlabel('Number of vertices')
    plt.ylabel('Time spent over n log n')
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
    format_axis()

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
    format_axis()

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
    basedir = 'c:\\Users\\Niels\\projects\\Thesis-Convex\\data\\'
    # plot_comparison(os.path.join(basedir, 'in_sphere_10m.csv'), 'Time for points in a sphere', 'time_qh_inc_in_sphere', 'Time spent')
    # plot_nlogn(os.path.join(basedir, 'in_sphere_10m.csv'), 'Time for points in a sphere over nlogn', 'time_qh_inc_in_sphere_nlogn', minrange=8)
    # plot_comparison(os.path.join(basedir, 'on_sphere_100k.csv'), 'Time for points on a sphere', 'time_qh_inc_on_sphere', 'Time spent')
    # plot_nlogn(os.path.join(basedir, 'on_sphere_100k.csv'), 'Time for points on a sphere over nlogn', 'time_qh_inc_on_sphere_nlogn', [0.0,10.5])
    # plot_nsquared(os.path.join(basedir, 'on_sphere_100k.csv'), 'Time for points on a sphere over nsquared', 'time_qh_inc_on_sphere_squard', [0.0,0.01], 5)
    # plot_comparison(os.path.join(basedir, 'in_cube_1_2m.csv'), 'Time for points in a cube', 'time_qh_inc_in_cube', 'Time spent')
    # plot_comparison(os.path.join(basedir, 'normalized_sphere.csv'), 'Time for points on a normalized sphere', 'time_qh_inc_normalized_sphere', 'Time spent')
    # plot_nsquared(os.path.join(basedir, 'normalized_sphere.csv'), 'Time for points on a normalized sphere over nsquared', 'time_qh_inc_normalized_squared', [0.0,0.02], 5)
    # plot_comparison(os.path.join(basedir, 'many_internal.csv'), 'Time for points on a distribution with many internal points', 'time_qh_inc_many_internal', 'Time spent')
    # plot_single(os.path.join(basedir, 'qh_many_internal.csv'), 'Time for points on a distribution with many internal points', 'time_qh_many_internal', 'QuickHull', 'Time spent')
    # plot_single_nlogn(os.path.join(basedir, 'qh_many_internal.csv'), 'Time for points on a distribution with many internal points', 'time_qh_many_internal', 'QuickHull', [0.0, 0.15], 5)
    # plot_comparison(os.path.join(basedir, 'on_sphere_processed.csv'), 'Number of points processed for points on a sphere', 'processed_qh_inc_on_sphere', 'Processed points')
    plot_three(os.path.join(basedir, 'many_internal_processed.csv'), 'Number of points processed and in hull when many points are internal', 'processed_internal', 'Points')