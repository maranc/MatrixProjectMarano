/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package test.simd;

/**
 *
 * @author Cujo
 */
public class main {

    public static void main(String args[]) {
        marix a= new marix();
        System.out.println("Able Auto SIMD by JVM");
        for (int i=100; i < 1000; i += 100) {
               double out = a.mmult(i);
               System.out.println(out);
        }
        System.out.println("not Able Auto SIMD by JVM");
        for (int i=100; i < 1000; i += 100) {
               double out = a.Non_mmult(i);
               System.out.println(out);
        }
    }
}
