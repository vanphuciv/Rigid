using UnityEngine;
using System.Collections;
using System;
using System.Runtime.InteropServices;

public class ParticleMotionScript : MonoBehaviour {
	int id;
	int f_id;
	int test;
	private IntPtr bird = IntPtr.Zero;
	float[] boid;
	Vector3 velocity;


	[DllImport("ParticlePlugin")]
	private static extern void createBird(ref IntPtr bird,float[] boid ,int id);

	[DllImport("ParticlePlugin")]
	private static extern void birdRun (ref IntPtr bird,float[] boid,int id);

	[DllImport("ParticlePlugin")]
	private static extern void destroyBird (ref IntPtr bird);

	//Use this for initialization
	void Start () {
		boid = new float[200*6];
		for(int i=0;i<200;i++){
			if (SpawnerScript.boids[i] != null) {
				if (SpawnerScript.boids[i] == gameObject) {
					id = i;
					f_id = id*6;
					boid [f_id] = transform.position[0];
					boid [1+f_id] = transform.position[1];
					boid [2+f_id] = transform.position[2];
					boid [3+f_id] = UnityEngine.Random.Range (-1, 1);
					boid [4+f_id] = UnityEngine.Random.Range (-1, 1);
					boid [5+f_id] = UnityEngine.Random.Range (-1, 1);
				}
			}
		}
	
		createBird (ref bird, boid, id);

	}
	
	void FixedUpdate () {
		//float dt = Time.deltaTime;
		birdRun (ref bird, boid, id);
		transform.position = new Vector3 (boid[f_id],boid[f_id+1],boid[f_id+2]);
		if ((test < 10)&&(id ==5)) {
			test++;
			//	Debug.Log(boid[4]);
			//	Debug.Log(boid[5]);
			Debug.Log(boid[id*6-6+5]);
		}
	}


	void OnDestroy (){
		destroyBird (ref bird);
	}
}
