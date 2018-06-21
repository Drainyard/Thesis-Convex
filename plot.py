import matplotlib
import os
from matplotlib import rcParams
rcParams['font.family'] = 'serif'
rcParams['font.sans-serif'] = ['Palatino']
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.ticker import FormatStrFormatter

from matplotlib import rcParams
rcParams.update({'figure.autolayout': True})

markers = {'Quickhull': 'x', 'Randomized incremental': '^', 'Divide and Conquer': 'p', 'Points on hull': 'h', 
            'In Sphere' : 'x', 'On Sphere' : '^', 'Normalized Sphere': 'p', 'Many Internal' : 'h', 'In Cube' : '*'}

def trailing_zeros(longint):
    manipulandum = str(longint)
    return len(manipulandum)-len(manipulandum.rstrip('0'))

def len_of_num(n):
    s = str(n)
    return len(s)

def format_num():
    return (lambda x, p: x)
    
def format_axis(ax, large_num=False):
    ax.get_xaxis().set_major_formatter(
        matplotlib.ticker.FuncFormatter(lambda x, p: 
            0 if x == 0 else '${0:.1f} \cdot 10^{1}$'.format(np.divide(x, np.power(10, len_of_num(int(x)) - 1)), len_of_num(int(x)) - 1)
        )
    )

    if large_num:
        ax.get_yaxis().set_major_formatter(
            matplotlib.ticker.FuncFormatter(lambda x, p: 
            0 if x == 0 else '${0:.1f} \cdot 10^{1}$'.format(np.divide(x, np.power(10, len_of_num(x) - 3)), len_of_num(x) - 3))
        )
    else:
        ax.yaxis.set_major_formatter
        (
            matplotlib.ticker.FuncFormatter(format_num())
        )

def plot_single(path, title, outfile, label, ylabel, show=False):
    data = np.genfromtxt(path, delimiter=',', names=['x', 'y'])

    
    plt.tight_layout()
    data['y'] = [np.divide(y, 1000000.0) for y in data['y']]

    fig, ax = plt.subplots()
    ax.plot(data['x'], data['y'], color='black', label=label, marker=markers[label], markersize=6.0, linewidth=0.5)
    format_axis(ax)
    ax.legend()

    plt.xlabel('n')
    plt.ylabel(ylabel)
    plt.title(title)
    savefig(outfile)
    if show:
        plt.show()

    plt.close()

def plot_comparison(path, title, outfile, ylabel, labels = ['Quickhull', 'Randomized incremental'], divide=True, yrange=None, minrange=0, percent=False, show=False):
    data = np.genfromtxt(path, delimiter=',', names=['x_qh', 'y_qh', 'x_inc', 'y_inc'])

    fig, ax = plt.subplots()
    plt.tight_layout()
    if divide:
        data['y_qh'] = [np.divide(y, 1000000.0) for y in data['y_qh']]
        data['y_inc'] = [np.divide(y, 1000000.0) for y in data['y_inc']]

    if labels != None:
        format_axis(ax, not divide)
        ax.plot(data['x_qh'][minrange:,], data['y_qh'][minrange:,], color='black', label=labels[0], marker=markers[labels[0]], markersize=6.0, linewidth=0.5)
        ax.plot(data['x_inc'][minrange:,], data['y_inc'][minrange:,], color='black', label=labels[1], marker=markers[labels[1]], markersize=6.0, linewidth=0.5, linestyle='dashed')
        ax.legend()
    else:
        xdat_qh = data['x_qh'][minrange:,]
        ydat_qh = data['y_qh'][minrange:,]
        xdat_inc = data['x_inc'][minrange:,]
        ydat_inc = data['y_inc'][minrange:,]
        l1 = labels[0] if labels != None else 'Quickhull'
        l2 = labels[1] if labels != None else 'Randomized incremental'

        ax.plot(xdat_qh, ydat_qh, color='black', marker=markers[l1], markersize=6.0, linewidth=0.5)
        ax.plot(xdat_inc, ydat_inc, color='black', marker=markers[l2], markersize=6.0, linewidth=0.5, linestyle='dashed')
        format_axis(ax, not divide)

    if percent:
        vals = ax.get_yticks()
        ax.set_yticklabels(['{:3.2f}%'.format(x) for x in vals])
    
    if yrange != None:
         ax.set_ylim(yrange)

    plt.xlabel('n')
    plt.ylabel(ylabel)
    plt.title(title)
    savefig(outfile)
    if show:
        plt.show()

    plt.close()

def plot_comparison_n(path, title, outfile, ylabel, labels=None, divide=False, show=False, minrange=0):
    label_len = len(labels)
    names = []
    # markers = ['x', '^', 'p', 'h', '*', 'o', '.', ',', '<', '>', 's']
    linetypes = ['-', '--', '-.', ':']
    for i in range(0, label_len):
        names.append('x_{}'.format(i))
        names.append('y_{}'.format(i))
    data = np.genfromtxt(path, delimiter=',', names=names)


    plt.tight_layout()

    fig, ax = plt.subplots()

    for i in range(0, len(labels)):
        x_name = 'x_{}'.format(i)
        y_name = 'y_{}'.format(i)
        if divide:
            data[y_name] = [np.divide(y, 1000000.0) for y in data[y_name]]
        ax.plot(data[x_name][minrange:,], data[y_name][minrange:,], color='black', label=labels[i], marker=markers[labels[i]], markersize=6.0, linewidth=0.5, linestyle=linetypes[i % len(linetypes)])

    format_axis(ax)
    ax.legend()

    plt.xlabel('n')
    plt.ylabel(ylabel)
    plt.title(title)
    savefig(outfile)

    if show:
        plt.show()

    plt.close()

def plot_three(path, title, outfile, ylabel, labels=['Quickhull', 'Randomized incremental', 'Points on hull'], divide=False, show=False):
    data = np.genfromtxt(path, delimiter=',', names=['x_qh', 'y_qh', 'x_inc', 'y_inc', 'x_out', 'y_out'])

    fig, ax = plt.subplots()
    plt.tight_layout()

    if divide:
        data['y_qh'] = [np.divide(y, 1000000.0) for y in data['y_qh']]
        data['y_inc'] = [np.divide(y, 1000000.0) for y in data['y_inc']]
        data['y_out'] = [np.divide(y, 1000000.0) for y in data['y_out']]

    if labels != None:
        ax.plot(data['x_qh'], data['y_qh'], color='black', label=labels[0], marker=markers[labels[0]], markersize=6.0, linewidth=0.5)
        ax.plot(data['x_inc'], data['y_inc'], color='black', label=labels[1], marker=markers[labels[1]], markersize=6.0, linewidth=0.5, linestyle='dashed')
        ax.plot(data['x_out'], data['y_out'], color='black', label=labels[2], marker=markers[labels[2]], markersize=6.0, linewidth=0.5, linestyle='dashdot')
        format_axis(ax)
        ax.legend()
    else:
        ax.plot(data['x_qh'], data['y_qh'], color='black', marker=markers[labels[0]], markersize=6.0, linewidth=0.5)
        ax.plot(data['x_inc'], data['y_inc'], color='black', marker=markers[labels[1]], markersize=6.0, linewidth=0.5, linestyle='dashed')
        ax.plot(data['x_out'], data['y_out'], color='black', marker=markers[labels[2]], markersize=6.0, linewidth=0.5, linestyle='dashdot')
        format_axis(ax)                    

    plt.xlabel('n')
    plt.ylabel(ylabel)
    plt.title(title)
    savefig(outfile)
    if show:
        plt.show()

    plt.close()

def plot_single_nlogn(path, title, outfile, label, miny = [0.0, 0.5], minrange=0, show=False):
    data = np.genfromtxt(path, delimiter=',', names=['x', 'y'])

    fig, ax = plt.subplots()
    plt.tight_layout()

    data['y'] = [np.divide(y, np.multiply(x, np.log2(x))) for y,x in zip(data['y'], data['x'])]
    ax.plot(data['x'][minrange:,], data['y'][minrange:,], color='black', label=label, marker=markers[label], markersize=6.0, linewidth=0.5)
    #ax.plot(data['x_inc'], data['y_inc'], color='black', label='data', marker='^', markersize=6.0)
    format_axis(ax)

    ax.set_ylim(miny)
    ax.legend()

    plt.xlabel('n')
    plt.ylabel('Time over n log n (s)')
    plt.title(title)
    savefig(outfile)
    if show:
        plt.show()

    plt.close()

def plot_nlogn(path, title, outfile, miny = [0.0, 0.5], minrange=0, show=False):
    data = np.genfromtxt(path, delimiter=',', names=['x_qh', 'y_qh', 'x_inc', 'y_inc'])

    fig, ax = plt.subplots()
    plt.tight_layout()

    data['y_qh'] = [np.divide(y, np.multiply(x, np.log2(x))) for y,x in zip(data['y_qh'], data['x_qh'])]
    data['y_inc'] = [np.divide(y, np.multiply(x, np.log2(x))) for y,x in zip(data['y_inc'], data['x_inc'])]
    ax.plot(data['x_qh'][minrange:,], data['y_qh'][minrange:,], color='black', label='Quickhull', marker=markers['Quickhull'], markersize=6.0, linewidth=0.5)
    ax.plot(data['x_inc'][minrange:,], data['y_inc'][minrange:,], color='black', label='Randomized incremental', marker=markers['Randomized incremental'], markersize=6.0, linewidth=0.5, linestyle='dashed')
    #ax.plot(data['x_inc'], data['y_inc'], color='black', label='data', marker='^', markersize=6.0)
    format_axis(ax)

    ax.set_ylim(miny)
    ax.legend()

    plt.xlabel('n')
    plt.ylabel('Time over n log n (s)')
    plt.title(title)
    savefig(outfile)
    if show:
        plt.show()

    plt.close()

def plot_nsquared(path, title, outfile, miny = [0.0, 0.5], minrange=0, show=False):
    data = np.genfromtxt(path, delimiter=',', names=['x_qh', 'y_qh', 'x_inc', 'y_inc'])

    fig, ax = plt.subplots()
    plt.tight_layout()

    data['y_qh'] = [np.divide(y, np.multiply(x, x)) for y,x in zip(data['y_qh'], data['x_qh'])]
    data['y_inc'] = [np.divide(y, np.multiply(x, x)) for y,x in zip(data['y_inc'], data['x_inc'])]
    ax.plot(data['x_qh'][minrange:,], data['y_qh'][minrange:,], color='black', label='Quickhull', marker='x', markersize=6.0, linewidth=0.5)
    ax.plot(data['x_inc'][minrange:,], data['y_inc'][minrange:,], color='black', label='Randomized incremental', marker='^', markersize=6.0, linewidth=0.5, linestyle='dashed')
    #ax.plot(data['x_inc'], data['y_inc'], color='black', label='data', marker='^', markersize=6.0)
    format_axis(ax)

    ax.set_ylim(miny)
    ax.legend()

    plt.xlabel('n')
    plt.ylabel('Time over n^2 (s)')
    plt.title(title)
    savefig(outfile)
    
    if show:
        plt.show()

    plt.close()

def plot_ncubed(path, title, outfile, miny = [0.0, 0.5], minrange=0, show=False):
    data = np.genfromtxt(path, delimiter=',', names=['x_qh', 'y_qh', 'x_inc', 'y_inc'])

    fig, ax = plt.subplots()
    plt.tight_layout()

    data['y_qh'] = [np.divide(y, np.multiply(np.multiply(x, x), x)) for y,x in zip(data['y_qh'], data['x_qh'])]
    data['y_inc'] = [np.divide(y, np.multiply(np.multiply(x, x), x)) for y,x in zip(data['y_inc'], data['x_inc'])]
    ax.plot(data['x_qh'][minrange:,], data['y_qh'][minrange:,], color='black', label='Quickhull', marker=markers['Quickhull'], markersize=6.0, linewidth=0.5)
    ax.plot(data['x_inc'][minrange:,], data['y_inc'][minrange:,], color='black', label='Randomized incremental', marker=markers['Randomized incremental'], markersize=6.0, linewidth=0.5, linestyle='dashed')
    #ax.plot(data['x_inc'], data['y_inc'], color='black', label='data', marker='^', markersize=6.0)
    format_axis(ax)

    ax.set_ylim(miny)
    ax.legend()

    plt.xlabel('n')
    plt.ylabel('Time over n^3 (s)')
    plt.title(title)
    savefig(outfile)
    
    if show:
        plt.show()

    plt.close()

def get_rgb(r, g, b, a = 255.0):
    return [r / 255.0, g / 255.0, b / 255.0, a / 255.0]

def plot_bar(title, outfile):
    
    qh_data = (84519, 157545, 8920)
    inc_data = (307792, 234444, 10587)
    div_data = (58661, 48141, 2020)

    fig, ax = plt.subplots()
    plt.tight_layout()

    ind = np.arange(3)
    width = 0.2

    qh_rect = ax.bar(ind, qh_data, width, edgecolor=get_rgb(0, 255.0, 55.0), color=get_rgb(151.0, 255.0, 174.0))
    inc_rect = ax.bar(ind + width + 0.1, inc_data, width, edgecolor=get_rgb(163.0, 71.0, 255.0), color=get_rgb(206.0, 157.0, 255.0))
    div_rect = ax.bar(ind + (width + 0.1) * 2, div_data, width, edgecolor=get_rgb(204.0, 204.0, 0.0), color=get_rgb(254.0, 255.0, 200.0))

    ax.set_ylabel('Time spent ($\mu$s)')
    ax.set_title('Time spent on real-world data')
    ax.set_xticks(ind + width / 2)
    ax.set_xticklabels(('Man in Vest \n(65536)', 'Arnold Schwarzenegger \n(16384)', 'Stanford Bunny \n(2048)'))

    ax.legend((qh_rect[0], inc_rect[0], div_rect[0]), ('Quickhull', 'Randomized incremental', 'Divide-and-conquer'))
    savefig(outfile)

    plt.show()


def savefig(path):
    plt.savefig(os.path.join('graphs' + os.sep, path + '.pdf'), format='pdf', dpi=1000)
    

if __name__ == '__main__':
    basedir = 'c:\\Users\\Niels\\projects\\Thesis-Convex\\data\\'
    # plot_comparison(os.path.join(basedir, 'in_sphere_10m.csv'), 'Time for points in a sphere', 'time_qh_inc_in_sphere', 'Time (s)')
    # plot_nlogn(os.path.join(basedir, 'in_sphere_10m.csv'), 'Time for points in a sphere over nlogn', 'time_qh_inc_in_sphere_nlogn', minrange=8)
    # plot_comparison(os.path.join(basedir, 'on_sphere_100k.csv'), 'Time for points on a sphere', 'time_qh_inc_on_sphere', 'Time (s)')
    # plot_nlogn(os.path.join(basedir, 'on_sphere_100k.csv'), 'Time for points on a sphere over nlogn', 'time_qh_inc_on_sphere_nlogn', [0.0,10.5])
    # plot_nsquared(os.path.join(basedir, 'on_sphere_100k.csv'), 'Time for points on a sphere over nsquared', 'time_qh_inc_on_sphere_squard', [0.0,0.01], 5)
    # plot_ncubed(os.path.join(basedir, 'on_sphere_100k.csv'), 'Time for points on a sphere over ncubed', 'time_qh_inc_on_sphere_cubed', [0.0,0.00000001], 5)
    # plot_comparison(os.path.join(basedir, 'in_cube_1_2m.csv'), 'Time for points in a cube', 'time_qh_inc_in_cube', 'Time (s)')
    # plot_nlogn(os.path.join(basedir, 'in_cube_1_2m.csv'), 'Time for points in a cube over nlogn', 'time_qh_inc_in_cube_nlogn', [0.0, 1.5], 7)
    # plot_comparison(os.path.join(basedir, 'normalized_sphere.csv'), 'Time for points on a normalized sphere', 'time_qh_inc_normalized_sphere', 'Time (s)')
    # plot_nsquared(os.path.join(basedir, 'normalized_sphere.csv'), 'Time for points on a normalized sphere over nsquared', 'time_qh_inc_normalized_squared', [0.0,0.02], 5)
    # plot_comparison(os.path.join(basedir, 'many_internal.csv'), 'Time for points on a distribution with many internal points', 'time_qh_inc_many_internal', 'Time (s)')
    # plot_single(os.path.join(basedir, 'qh_many_internal.csv'), 'Time for points on a distribution with many internal points', 'time_qh_many_internal', 'Quickhull', 'Time (s)')
    # plot_single_nlogn(os.path.join(basedir, 'qh_many_internal.csv'), 'Time for points on a distribution with many internal points over nlogn', 'time_qh_many_internal_nlogn', 'Quickhull', [0.0, 0.15], 5)
    # plot_comparison(os.path.join(basedir, 'on_sphere_processed.csv'), 'Number of points processed for points on a sphere', 'processed_qh_inc_on_sphere', 'Processed points', divide=False)
    # plot_three(os.path.join(basedir, 'many_internal_processed.csv'), 'Number of points processed and in hull when many points are internal', 'processed_internal', 'Points', divide=False)
    # plot_comparison(os.path.join(basedir, 'normalized_qh.csv'), 'Time on sphere and on normalized sphere (Quickhull)', 'time_qh_on_norm_sphere', 'Time (s)', labels=None)
    # plot_three(os.path.join(basedir, 'in_sphere_dac_inc_qh.csv'), 'Time for points in a sphere', 'in_sphere_dac_inc_qh', 'Time (s)', ['Quickhull', 'Randomized incremental', 'Divide and Conquer'], divide=True)
    # plot_three(os.path.join(basedir, 'on_sphere_dac_inc_qh.csv'), 'Time for points on a sphere', 'on_sphere_dac_inc_qh', 'Time (s)', ['Quickhull', 'Randomized incremental', 'Divide and Conquer'], divide=True)
    # plot_comparison(os.path.join(basedir, 'on_sphere_inc_dac.csv'), 'Time for points on a sphere', 'on_sphere_inc_dac', 'Time (s)', labels=['Randomized incremental', 'Divide and Conquer'])
    # plot_three(os.path.join(basedir, 'in_cube_dac_inc_qh.csv'), 'Time for points in a cube', 'in_cube_dac_inc_qh', 'Time (s)', ['Quickhull', 'Randomized incremental', 'Divide and Conquer'], divide=True)
    # plot_comparison(os.path.join(basedir, 'faces_on_hull_in_sphere_qh_inc.csv'), 'Percentage of created faces on the hull', 'faces_on_hull_in_sphere', 'Faces on hull (%)', labels=['Quickhull', 'Randomized incremental'], divide=False, yrange=[5.0, 25.0], minrange=5, percent=True)
    # plot_comparison(os.path.join(basedir, 'faces_on_hull_normalized_sphere_qh_inc.csv'), 'Percentage of created faces on the hull', 'faces_on_hull_on_norm_sphere', 'Faces on hull (%)', labels=['Quickhull', 'Randomized incremental'], divide=False, yrange=[32.0, 35.0], minrange=5, percent=True)
    # plot_comparison(os.path.join(basedir, 'faces_on_hull_on_sphere_qh_inc.csv'), 'Percentage of created faces on the hull', 'faces_on_hull_on_sphere', 'Faces on hull (%)', labels=['Quickhull', 'Randomized incremental'], divide=False, yrange=[32.5, 34.0], minrange=5, percent=True)
    # plot_comparison_n(os.path.join(basedir, 'dac_nlogn.csv'), 'Time (s) over nlogn', 'time_dac_nlogn', 'Time (s) over nlogn', labels=['In Sphere', 'On Sphere', 'In Cube', 'Normalized Sphere', 'Many Internal'], minrange=4) 
    # plot_comparison(os.path.join(basedir, 'sidedness_in_sphere_qh_inc.csv'), 'Sidedness queries', 'sidedness_in_sphere_qh_inc', 'Sidedness queries', labels=['Quickhull', 'Randomized incremental'], divide=False)
    # plot_comparison(os.path.join(basedir, 'sidedness_norm_sphere_qh_inc.csv'), 'Sidedness queries', 'sidedness_norm_sphere_qh_inc', 'Sidedness queries', labels=['Quickhull', 'Randomized incremental'], divide=False, show=True)
    # plot_comparison(os.path.join(basedir, 'sidedness_many_internal_qh_inc.csv'), 'Sidedness queries', 'sidedness_many_internal_qh_inc', 'Sidedness queries', labels=['Quickhull', 'Randomized incremental'], divide=False, show=True)
    # plot_comparison(os.path.join(basedir, 'sidedness_on_sphere_qh_inc.csv'), 'Sidedness queries', 'sidedness_on_sphere_qh_inc', 'Sidedness queries', labels=['Quickhull', 'Randomized incremental'], divide=False, show=True)
    # plot_comparison(os.path.join(basedir, 'sidedness_in_cube_qh_inc.csv'), 'Sidedness queries', 'sidedness_in_cube_qh_inc', 'Sidedness queries', labels=['Quickhull', 'Randomized incremental'], divide=False, show=True)
    plot_bar('Model comparison', 'bar_models')