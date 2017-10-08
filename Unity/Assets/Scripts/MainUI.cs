using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class MainUI : MonoBehaviour {
	[SerializeField] Slider redSlider;
	[SerializeField] Slider blueSlider;
	[SerializeField] Text turnText;
    [SerializeField] float reachSecond = 1.0f;
	[SerializeField] float gageSecond = 0.5f;

	public void AnnounceRedTurn(){
		turnText.gameObject.SetActive (true);
		turnText.color = Color.red;
		turnText.text = "Red Turn";
        StartCoroutine(ScrollTurnTextCorutine());
	}

	public void AnnounceBlueTurn(){
		turnText.gameObject.SetActive (true);
		turnText.color = Color.blue;
		turnText.text = "Blue Turn";
		StartCoroutine(ScrollTurnTextCorutine());
	}

    IEnumerator ScrollTurnTextCorutine(){
        Vector3 rightPosition = new Vector3(this.GetComponent<RectTransform>().sizeDelta.x / 2 + turnText.rectTransform.sizeDelta.x, 0, 0);
        float second = 0;
        while(second < reachSecond){
            turnText.transform.localPosition = Vector3.Lerp(rightPosition, Vector3.zero, second / reachSecond);
            second += Time.deltaTime;
			yield return null;
        }
        yield return new WaitForSeconds(1.0f);
        Vector3 leftPosition = new Vector3(-rightPosition.x, 0 , 0);
		second = 0;
		while (second < reachSecond)
		{
            turnText.transform.localPosition = Vector3.Lerp(Vector3.zero, leftPosition, second / reachSecond);
			second += Time.deltaTime;
			yield return null;
		}
        turnText.gameObject.SetActive(false);
    }

	public void ChangeRedSlider(float sliderValue){
		Debug.Log (sliderValue);
		redSlider.value = sliderValue;
	}

	public void ChangeBlueSlider(float sliderValue){
		Debug.Log (sliderValue);
		blueSlider.value = sliderValue;
	}

	public IEnumerator RedSliderScroll(float sliderValue)
	{
		float currentValue = redSlider.value;
		float second = 0;
		while (second < gageSecond)
		{
			redSlider.value = sliderValue + ((currentValue - sliderValue) * ((gageSecond - second) / gageSecond));
			second += Time.deltaTime;
			yield return null;
		}
		redSlider.value = sliderValue;
	}

    public IEnumerator BlueSliderScroll(float sliderValue)
	{
        float currentValue = blueSlider.value;
        float second = 0;
        while (second < gageSecond)
        {
            blueSlider.value = sliderValue + ((currentValue - sliderValue) * ((gageSecond - second) / gageSecond));
			second += Time.deltaTime;
            yield return null;
        }
		blueSlider.value = sliderValue;
	}
}