using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class BattleCar : MonoBehaviour {
	private BattleCar opponentCar;
	private Vector3 defaultPosition;
	private Vector3 defaultVelocity;
	private Rigidbody rigidbody;

	void Start(){
		defaultPosition = this.transform.position;
		rigidbody = GetComponent<Rigidbody> ();
		defaultVelocity = rigidbody.velocity;
	}

	public void SetOpponentCar(BattleCar car){
		opponentCar = car;
	}

	public void Attack(float speed){
		rigidbody.AddForce(new Vector3(1,0,0) * speed, ForceMode.Impulse);
		rigidbody.velocity = (new Vector3(1,0,0) * speed) / rigidbody.mass;
	}

	public void Reset(){
		this.transform.position = defaultPosition;
		rigidbody.velocity = defaultVelocity;
	}
}
